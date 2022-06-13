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

#ifndef POWER_SAVER_H_
#define POWER_SAVER_H_

#include "debug.h"
#include <Arduino.h>

typedef enum {
        kPslsRunning,
        kPslsWarning,
        kPslsTimeout,
        kPslsSleep,
} PowerSaverLoopState;

class PowerSaver {

private:
uint64_t returnToMainTimeout = 9000;     // Time after last user input that the menu switches back to main menu.
uint64_t sleepTimeout = 12000;     // Timer after last user input to go to sleep mode (display off also).
uint64_t sleepTimer;

void (*rtmtCallback)(void);
void (*stCallback)(void);
PowerSaverLoopState loopState;


public:

PowerSaver(void) {
}

void setup(uint16_t returnToMainTimeout, uint16_t sleepTimer, void (*rtmtCallback)(void), void (*stCallback)(void));
void changeLoopState(PowerSaverLoopState newState);
void loop(void);

void resetTimer(void);
};

#endif // POWER_SAVER_H_
