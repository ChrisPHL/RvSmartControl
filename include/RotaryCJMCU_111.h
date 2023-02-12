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

#ifndef ROTARY_CJMCU_111_H_
#define ROTARY_CJMCU_111_H_

#include "debug.h"
#include <Arduino.h>


void isrGx(void);


const uint8_t isrGxDebounceDuration = 3; // msec

const uint16_t isrGxLongPressDuration = 1000; // msec


class RotaryCJMCU_111
{
  private:

  RotaryCJMCU_111(void) {
          pinGaShortToAnalog = false;
  }     // verhindert, dass ein Objekt von außerhalb von RotaryCJMCU_111 erzeugt wird.
  // protected, wenn man von der Klasse noch erben möchte
  RotaryCJMCU_111( const RotaryCJMCU_111& );     // verhindert, dass eine weitere Instanz via Kopier-Konstruktor erstellt werden kann
  RotaryCJMCU_111 & operator = (const RotaryCJMCU_111 &);     // Verhindert weitere Instanz durch Kopie


  void setup(void) {
        resetRotaryInput();

        pinMode(rotaryPinGa, INPUT_PULLUP);
        digitalWrite(rotaryPinGa, HIGH);
        pinMode(rotaryPinGb, INPUT_PULLUP);
        digitalWrite(rotaryPinGb, HIGH);

        if (pinGaShortToAnalog) {
                analogReadResolution(12);
                analogSetWidth(12);
                analogSetCycles(8);
                analogSetSamples(1);
                analogSetClockDiv(1);
        }
        attachInterrupt(digitalPinToInterrupt(rotaryPinGa), isrGx, CHANGE);
        attachInterrupt(digitalPinToInterrupt(rotaryPinGb), isrGx, CHANGE);
  }

  public:
  static RotaryCJMCU_111& getInstance() {
          static RotaryCJMCU_111 instance;
          return instance;
  }

  volatile uint8_t rotaryPinGa;
  volatile uint8_t rotaryPinGb;
  volatile bool pinGaShortToAnalog;
  volatile uint8_t rotaryPinGaAnalog;

  void setup(uint8_t pinGa, uint8_t pinGb);
  void setup(uint8_t pinGa, uint8_t pinGb, uint8_t pinGaAnalog);

  void loop(void);

  bool isRotaryInputDetected(void);
  uint32_t getRotationCountL(void);
  uint32_t getRotationCountR(void);
  uint32_t getRotaryPushCount(void);
  uint32_t getRotaryLongPressCount(void);
  void resetRotaryInput(void);

  bool isButtonPushed(void);
};


#endif // ROTARY_CJMCU_111_H_
