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

#ifndef MENU_BATTERY_STATUS_H_
#define MENU_BATTERY_STATUS_H_

#include "debug.h"
#include <Arduino.h>

#include <Adafruit_ILI9341.h>

class MenuBatteryStatus {

private:

protected:

public:

MenuBatteryStatus() {
}

virtual void updateBatteryIndicator(uint16_t batteryX, uint16_t batteryY, uint16_t batteryWidth, uint16_t batteryHeight);
virtual void drawBatteryIndicator(uint16_t x, uint16_t y, uint8_t w, uint8_t h, uint8_t poleH, uint8_t poleW);

};

#endif // MENU_BATTERY_STATUS_H_
