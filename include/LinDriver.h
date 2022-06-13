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

#ifndef LIN_DRIVER_H_
#define LIN_DRIVER_H_

#include <Arduino.h>
#include <HardwareSerial.h>

#define LIN_SERIAL HardwareSerial

const uint8_t LIN_BREAK_DURATION = 15;    // Number of bits in the break.
const uint8_t LIN_TIMEOUT_IN_FRAMES = 2;     // Wait this many max frame times before declaring a read timeout.

const uint8_t LIN_HEADER_BREAK = 0x80;
const uint8_t LIN_HEADER_SYNC = 0x55;

// This code allows you to derive from the LINFrame class to define messages of different lengths and therefore save a few bytes of RAM.  But it relies on the compiler laying out the class memory in RAM so that the derived class data directly follows the base class.
// If this is not the case in your system, the easiest way to get something working is to only allow full frames, as shown in the #else clause.
// Also, you can over-malloc the LINFrame class to create an extended data buffer.  Of course this method has its own memory management overhead.

enum
{
  Lin1Frame = 0,
  Lin2Frame = 1,
  LinWriteFrame  = 0,
  LinReadFrame   = 2,
};


class LinDriver
{
  protected:
    void headerBreak(void);
    // For Lin 1.X "start" should = 0, for Lin 2.X "start" should be the addr byte.
    uint8_t dataChecksum(const uint8_t* message, char nBytes, uint16_t start = 0);
    uint8_t addrParity(uint8_t addr);

  public:
    LinDriver(LIN_SERIAL* serial, uint8_t txePin, uint8_t rxPin, uint8_t txPin, uint32_t baudrate = 19200);
    LIN_SERIAL* serial;
    uint8_t rxPin;               //  what pin # is used to receive (ESP32 is able to freely configure IO pins.)
    uint8_t txPin;               //  what pin # is used to transmit (needed to generate the BREAK signal)
    uint8_t txePin;              //  Pin for "transmit enable" (Need to be pulled down for receive data. )
    uint32_t baudrate;           //  in bits/sec. Also called baud rate
    uint8_t serialOn;            //  whether the serial port is "begin"ed or "end"ed.  Optimization so we don't begin twice.
    unsigned long int timeout;   //  How long to wait for a slave to fully transmit when doing a "read".  You can modify this after calling "begin"

    void begin(void);

    // Send a message right now, ignoring the schedule table.
    void send(uint8_t addr, const uint8_t* message, uint8_t nBytes, uint8_t proto = 2);

    // Receive a message right now, returns 0xff if good checksum, # bytes received (including checksum) if checksum is bad.
    uint8_t recv(uint8_t addr, uint8_t* message, uint8_t nBytes, uint8_t proto = 2);

    void busSleep(void);

    inline uint8_t bitExtract(uint8_t data, uint8_t shift) {
      return ((data&(1<<shift))>>shift);
    };
};

#endif // LIN_DRIVER_H_
