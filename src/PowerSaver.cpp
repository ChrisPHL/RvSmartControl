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

#include "PowerSaver.h"


void PowerSaver::setup(uint16_t returnToMainTimeout, uint16_t sleepTimer, void (*rtmtCallback)(void), void (*stCallback)(void)) {
        this->returnToMainTimeout = (uint64_t)returnToMainTimeout * (uint64_t)1000;
        this->sleepTimeout = (uint64_t)sleepTimer * (uint64_t)1000;
        this->rtmtCallback = rtmtCallback;
        this->stCallback = stCallback;

        loopState = kPslsRunning;

        resetTimer();
}

void PowerSaver::changeLoopState(PowerSaverLoopState newState) {
        loopState = newState;
}

void PowerSaver::loop(void) {

        switch (loopState) {
        case kPslsRunning:
                if (millis() - sleepTimer > returnToMainTimeout) {
                        rtmtCallback();
                        changeLoopState(kPslsWarning);
                }
                break;

        case kPslsWarning:
                if (millis() - sleepTimer > sleepTimeout) {
                        stCallback();
                        changeLoopState(kPslsTimeout);
                }
                break;

        case kPslsTimeout:
                // Just waiting for a reset here;
                break;

        default:
                resetTimer();
                break;
        }

}

void PowerSaver::resetTimer(void) {
        sleepTimer = millis();
        changeLoopState(kPslsRunning);
}
