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

#include "MainMenu.h"
#include "roadcruiser_xl.c"
#include "tcombi.c"
#include "airHeater.c"
#include "waterHeater.c"
#include "temperature.c"

// const uint16_t batteryX = 10; // upper left X
// static uint16_t batteryY = 25; // upper left Y
// const uint16_t batteryW = 80;
// const uint16_t batteryH = 150;
// const uint16_t batteryPoleW = 16; // battery pole hight
// const uint16_t batteryPoleH = 8; // battery pole width
const uint16_t batteryX = 112; // upper left X
static uint16_t batteryY = 125; // upper left Y
const uint16_t batteryW = 55;
const uint16_t batteryH = 40;
const uint16_t batteryPoleW = 12; // battery pole hight
const uint16_t batteryPoleH = 5; // battery pole width


void MainMenu::printScreenImplementation(void) {
        commonPrintScreen();

        drawCompressedImage(5, 50, reinterpret_cast<const RleImage*>(&roadcruiser_xl));
        drawCompressedImage(190, 65, reinterpret_cast<const RleImage*>(&airHeater));
        drawCompressedImage(190, 95, reinterpret_cast<const RleImage*>(&waterHeater));
        drawCompressedImage(172, 125, reinterpret_cast<const RleImage*>(&tcombi));
        drawCompressedImage(115, 65, reinterpret_cast<const RleImage*>(&temperature));

        menuBattStat->drawBatteryIndicator(batteryX, batteryY, batteryW, batteryH, batteryPoleH, batteryPoleW);

        // for (uint16_t i = 0; i <= 100; i++) {
        //         adaIli9431->fillRect(10 + i*3, 20, 3, 20, getColorGradient(ILI9341_RED, ILI9341_BLACK, i));
        //         adaIli9431->fillRect(10 + i*3, 40, 3, 20, getColorGradient(ILI9341_RED, ILI9341_WHITE, i));
        //         adaIli9431->fillRect(10 + i*3, 60, 3, 20, getColorGradient(ILI9341_GREEN, ILI9341_BLACK, i));
        //         adaIli9431->fillRect(10 + i*3, 80, 3, 20, getColorGradient(ILI9341_GREEN, ILI9341_WHITE, i));
        //         adaIli9431->fillRect(10 + i*3, 100, 3, 20, getColorGradient(ILI9341_BLUE, ILI9341_BLACK, i));
        //         adaIli9431->fillRect(10 + i*3, 120, 3, 20, getColorGradient(ILI9341_BLUE, ILI9341_WHITE, i));
        //         adaIli9431->fillRect(10 + i*3, 140, 3, 20, getColorGradient(ILI9341_WHITE, ILI9341_BLACK, i));
        //         adaIli9431->fillRect(10 + i*3, 160, 3, 20, getColorGradient(ILI9341_BLACK, ILI9341_WHITE, i));
        // }
}

void MainMenu::updateScreenImplementation(void) {
        menuBattStat->updateBatteryIndicator(batteryX, batteryY, batteryW, batteryH);
}

void MainMenu::onEnterButtonImplementation(void) {

}

void MainMenu::onLeaveButtonImplementation(void) {

}

void MainMenu::inputLeft(void) {

}

void MainMenu::inputRight(void) {

}

void MainMenu::inputPush(void) {

}
