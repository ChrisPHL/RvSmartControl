/*
   Copyright 2011 G. Andrew Stone
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LinDriver.h"
#include "debug.h"


LinDriver::LinDriver(LIN_SERIAL* serial, uint8_t txePin, uint8_t rxPin, uint8_t txPin, uint32_t baudrate) {
        this->serial = serial;
        this->txePin = txePin;

        this->baudrate = baudrate;
        this->rxPin = rxPin;
        this->txPin = txPin;
}

void LinDriver::begin(void) {
        serial->begin(baudrate, SERIAL_8N1, rxPin, txPin);
        serialOn  = 1;

        uint64_t Tbit = 100000 / baudrate; // Not quite in uSec, I'm saving an extra 10 to change a 1.4 (40%) to 14 below...
        uint64_t nominalFrameTime = ((34 * Tbit) + 90 * Tbit); // 90 = 10*max # payload bytes + checksum (9).
        timeout = LIN_TIMEOUT_IN_FRAMES * 14 * nominalFrameTime; // 14 is the specced addtl 40% space above normal*10 -- the extra 10 is just pulled out of the 1000000 needed to convert to uSec (so that there are no decimal #s).
        pinMode (txePin, OUTPUT);
        digitalWrite(txePin, HIGH);
}


// Generate a BREAK signal (a low signal for longer than a byte) across the serial line
void LinDriver::headerBreak(void)
{
        uint64_t brkend = (1000000UL / ((uint64_t)baudrate));
        uint64_t brkbegin = brkend * LIN_BREAK_DURATION;

#ifdef ESP32 // ESP32 does not support digital output on Tx pin so we're faking the signal the other way...
        serial->updateBaudRate(baudrate / 2);
        serial->write(LIN_HEADER_BREAK);
        delayMicroseconds(brkbegin);
        serial->updateBaudRate(baudrate);
#else // It's assumed to be an ESP8266 or Arduino:
        if (serialOn) serial->end();

        pinMode(txPin, OUTPUT);
        digitalWrite(txPin, LOW);  // Send BREAK
        if (brkbegin > 16383) delay(brkbegin / 1000); // delayMicroseconds unreliable above 16383 see arduino man pages
        else delayMicroseconds(brkbegin);

        digitalWrite(txPin, HIGH);  // BREAK delimiter

        if (brkend > 16383) delay(brkend / 1000); // delayMicroseconds unreliable above 16383 see arduino man pages
        else delayMicroseconds(brkend);

        serial->begin(serialSpd);
        serialOn = 1;
#endif
}

/* Lin defines its checksum as an inverted 8 bit sum with carry */
uint8_t LinDriver::dataChecksum(const uint8_t* message, char nBytes, uint16_t sum)
{
        while (nBytes-- > 0) sum += *(message++);
        // Add the carry
        while (sum >> 8) // In case adding the carry causes another carry
                sum = (sum & 255) + (sum >> 8);
        return (~sum);
}

/* Create the Lin ID parity */
uint8_t LinDriver::addrParity(uint8_t addr)
{
        uint8_t p0 = bitExtract(addr, 0) ^ bitExtract(addr, 1) ^ bitExtract(addr, 2) ^ bitExtract(addr, 4);
        uint8_t p1 = ~(bitExtract(addr, 1) ^ bitExtract(addr, 3) ^ bitExtract(addr, 4) ^ bitExtract(addr, 5));
        return (p0 | (p1 << 1)) << 6;
}

/* Send a message across the Lin bus */
void LinDriver::send(uint8_t addr, const uint8_t* message, uint8_t nBytes, uint8_t proto)
{
        uint8_t addrbyte = (addr & 0x3f) | addrParity(addr);
        // LIN diagnostic frame shall always use CHKSUM of protocol version 1.
        uint8_t cksum = dataChecksum(message, nBytes, (proto == 1 || addr == 0x3C) ? 0 : addrbyte);
        headerBreak(); // Generate the low signal that exceeds 1 char.
        serial->write(LIN_HEADER_SYNC);
        serial->write(addrbyte); // ID byte
        serial->write(message, nBytes); // data bytes
        serial->write(cksum); // checksum
}


uint8_t LinDriver::recv(uint8_t addr, uint8_t* message, uint8_t nBytes, uint8_t proto)
{
        uint8_t bytesRcvd = 0;
        unsigned int timeoutCount = 0;
        // Empty receive buffer:
        while (0 < serial->available()) {
                serial->read();
        }
        // Now start sending the request.
        headerBreak(); // Generate the low signal that exceeds 1 char.
        serial->write(LIN_HEADER_SYNC); // Sync byte
        uint8_t idByte = (addr & 0x3f) | addrParity(addr);
        serial->write(idByte); // ID byte
        serial->flush();
        digitalWrite(txePin, LOW); // don't pull up

        for (uint8_t i = 0; i < nBytes; i++) {
                // This while loop strategy does not take into account the added time for the logic.  So the actual timeout will be slightly longer then written here.
                while (!serial->available()) {
                        delayMicroseconds(100);
                        timeoutCount += 100;
                        if (timeoutCount >= timeout) goto done;
                }
                uint8_t byte = serial->read();
                if (!(0 == bytesRcvd && (LIN_HEADER_BREAK == byte || LIN_HEADER_SYNC == byte))) {
                        message[i] = byte;
                        bytesRcvd++;
                }
        }
        while (!serial->available()) {
                delayMicroseconds(100);
                timeoutCount += 100;
                if (timeoutCount >= timeout) goto done;
        }
        if (serial->available()) {
                uint8_t cksum = serial->read();
                bytesRcvd++;
                // LIN diagnostic frame shall always use CHKSUM of protocol version 1.
                if (proto == 1 || addr == 0x3D) {
                        idByte = 0; // Don't cksum the ID byte in LIN 1.x
                }
                if (dataChecksum(message, nBytes, idByte) == cksum) {
                        bytesRcvd = 0xff;
                }
        }
done:
        digitalWrite(txePin, HIGH);
        return bytesRcvd;
}

void LinDriver::busSleep(void) {
        uint8_t sleepCommandFrame[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        send(0x3C, sleepCommandFrame, 8);
}
