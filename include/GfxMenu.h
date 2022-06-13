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

#ifndef GFX_MENU_H_
#define GFX_MENU_H_


#include "debug.h"
#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "LinDriver.h"
#include "MenuItem.h"

typedef enum {
        kGmlsPrepareBootLogo,
        kGmlsShowBootLogo,
        kGmlsEnterMenu,
        kGmlsUpdateMenu,
} GfxMenuLoopState;


class GfxMenu {
private:
Adafruit_ILI9341* adaIli9431;
MenuItem* currentMenu;
uint8_t menuItemCount;

uint8_t pinChipSelect;
uint8_t pinDataCommand;
uint8_t pinReset;
uint8_t pinBacklight;
uint8_t pinPower;
uint64_t lastMenuCountUpdate;
uint8_t lastMenuItemCount;

void printBootLogo(void);
void printMenuScrollbar(void);
void updateMenuScrollbar(void);
void updateMenuCount(void);
uint8_t getCurrentMenuIndex(void);

public:
/**
   @param pinCs Pin for Chip Select.
   @param pinDc Pin for Data/Command.
   @param pinRst Reset Pin.
   @param pinPwr Pin for powering for display unit.
 */
GfxMenu(uint8_t pinCs, uint8_t pinDc, uint8_t pinRst, uint8_t pinBacklight = 0, uint8_t pinPwr = 0);
~GfxMenu(void);

void setup(LinDriver* linDriver = 0);

/**
   Call this method as fast as possible got guarantee fluent operation.
 */
void loop(void);

uint16_t lastLoopStateChange;
GfxMenuLoopState loopState;

void changeLoopState(GfxMenuLoopState newState);

void inputLeft(uint8_t count);
void inputRight(uint8_t count);
void inputPush(uint8_t count);
void inputLongPress(uint8_t count);

/**
   Call this method when menu timeout occurred. This shall switch back to system overview menu.
 */
void timeout(void);

void tftPowerUp(void);
void tftPowerDown(void);
};

#endif // GFX_MENU_H_
