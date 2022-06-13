/**
   This file is part of the "RV Smart Control" distribution
   (https://github.com/ChrisPHL/RvSmartControl).
   Copyright (c) 2021 Christian Pohl, aka ChrisPHL, www.chpohl.de.
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

   Addition: No commercial use without the explicit permission of the author.
 */

#include "RotaryCJMCU_111.h"


RotaryCJMCU_111& rotaryCjmcu111 = RotaryCJMCU_111::getInstance();


const uint32_t kRotationStatusWaiting         = 0x00000001; // Ga: HIGH and Gb: HIGH
const uint32_t kRotationStatusStartLorPush    = 0x00000002; // Ga: FALLING, Gb: HIGH
const uint32_t kRotationStatusStartR          = 0x00000003; // Ga: HIGH, Gb: FALLING
const uint32_t kRotationStatusTurningL        = 0x00000004; // Ga: RISING, Gb: LOW
const uint32_t kRotationStatusTurningR        = 0x00000005; // Ga: LOW, Gb: RISING
const uint32_t kRotationStatusWaitForFinishL  = 0x00000006; // Ga: RISING, Gb: LOW
const uint32_t kRotationStatusWaitForFinishR  = 0x00000007; // Ga: LOW, Gb RISING

volatile uint32_t isrGxTime = 0UL;
volatile uint32_t isrGxLastTime = 0UL;
volatile uint32_t rotationStatus = kRotationStatusWaiting;
volatile uint32_t rotaryInputDetected = 0UL;
volatile uint32_t rotationCountL = 0UL;
volatile uint32_t rotationCountR = 0UL;
volatile uint32_t rotaryPushCount = 0UL;
volatile uint32_t rotaryLongPressCount = 0UL;

void RotaryCJMCU_111::setup(uint8_t pinGa, uint8_t pinGb) {
        rotaryPinGa = pinGa;
        rotaryPinGb = pinGb;

        rotationStatus = kRotationStatusWaiting;
        setup();
}

void RotaryCJMCU_111::setup(uint8_t pinGa, uint8_t pinGb, uint8_t pinGaAnalog) {
        rotaryPinGaAnalog = pinGaAnalog;
        pinGaShortToAnalog = true;

        setup(pinGa, pinGb);
}


void RotaryCJMCU_111::loop(void) {
        if (kRotationStatusStartLorPush == rotationStatus) {
                if ((millis() - isrGxTime) >= isrGxLongPressDuration) {
                        isrGxTime = millis(); // Just count one LP <isrGxLongPressDuration>.
                        ++rotaryLongPressCount;
                        rotaryInputDetected = 1UL;
                }
        }
}

bool RotaryCJMCU_111::isRotaryInputDetected(void) {
        return (0UL == rotaryInputDetected ? false : true);
}

uint32_t RotaryCJMCU_111::getRotationCountL(void) {
        return rotationCountL;
}

uint32_t RotaryCJMCU_111::getRotationCountR(void) {
        return rotationCountR;
}

uint32_t RotaryCJMCU_111::getRotaryPushCount(void) {
        return rotaryPushCount;
}

uint32_t RotaryCJMCU_111::getRotaryLongPressCount(void){
        return rotaryLongPressCount;
}

void RotaryCJMCU_111::resetRotaryInput(void) {
        noInterrupts();
        rotaryInputDetected = 0UL;
        rotationCountL = 0UL;
        rotationCountR = 0UL;
        rotaryPushCount = 0UL;
        if (kRotationStatusStartLorPush != rotationStatus) {
                // Still counting long press...
                rotaryLongPressCount = 0UL;
        }
        interrupts();
}

bool RotaryCJMCU_111::isButtonPushed(void) {
        return (0 == digitalRead(rotaryPinGa) ? true : false);
}

/**
   ISR to catch rotary input events.
   Please note: https://www.esp8266.com/wiki/doku.php?id=esp8266_memory_map
   "SPI Flash is mapped here. Mapping hardware has 32KBytes cache (Unconfirmed). Non-cached code runs 12…13 times slower than code from IRAM. Cached code runs as fast as from IRAM. This area is readable as data with aligned 4-byte reads."
   ...with aligned 4-byte reads: THIS MEANS ONE HAS TO READ _OR_ WRITE (U)INT32_T DATA TYPES WITHIN ISR ONLY.
 */
void ICACHE_RAM_ATTR isrGx(void) {
        /*
           This routine relies on not being interrupted by another (higher prioritzed) interrupt.
           This should be the case for the Arduino hardware.
         */
        noInterrupts();
        // Debug.trigger(1);

        volatile uint32_t rotationInputA = digitalRead(rotaryCjmcu111.rotaryPinGa);
        volatile uint32_t rotationInputB = digitalRead(rotaryCjmcu111.rotaryPinGb);

        isrGxTime = millis();


        switch (rotationStatus) {
        case kRotationStatusWaiting:
                if (HIGH == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusStartR;
                }
                else if (LOW == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusStartLorPush;
                        isrGxLastTime = isrGxTime; // debounce
                }
                break;

        case kRotationStatusStartLorPush:
                if (HIGH == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusWaiting;
                        // TODO: if (rotaryCjmcu111.rotaryPinGaAnalog between 1.0 and 1.5 volts, then increment push count.
                        if ((isrGxTime - isrGxLastTime) > isrGxDebounceDuration) {
                                if ((isrGxTime - isrGxLastTime) < isrGxLongPressDuration) {
                                        ++rotaryPushCount;
                                }
                                // Debug.trigger(3);
                                rotaryInputDetected = 1UL;
                        }
                }
                else if (LOW == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusTurningL;
                }
                break;

        case kRotationStatusStartR:
                if (HIGH == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusWaiting;
                }
                else if (LOW == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusTurningR;
                }
                break;

        case kRotationStatusTurningL:
                if (LOW == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusStartLorPush;
                }
                else if (HIGH == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusWaitForFinishL;
                }
                break;

        case kRotationStatusTurningR:
                if (HIGH == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusStartR;
                }
                else if (LOW == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusWaitForFinishR;
                }
                break;

        case kRotationStatusWaitForFinishL:
                if (LOW == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusTurningL;
                }
                else if (HIGH == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusWaiting;
                        rotationCountR = 0UL;
                        ++rotationCountL;
                        rotaryInputDetected = 1UL;
                        // Debug.trigger(4);
                        isrGxLastTime = isrGxTime;         // debounce
                }
                break;

        case kRotationStatusWaitForFinishR:
                if (LOW == rotationInputA && LOW == rotationInputB) {
                        rotationStatus = kRotationStatusTurningR;
                }
                else if (HIGH == rotationInputA && HIGH == rotationInputB) {
                        rotationStatus = kRotationStatusWaiting;
                        rotationCountL = 0UL;
                        ++rotationCountR;
                        rotaryInputDetected = 1UL;
                        // Debug.trigger(5);
                        isrGxLastTime = isrGxTime;
                }
                break;
        }
        // Debug.trigger(2);
        interrupts();
}
