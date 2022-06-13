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

#include "debug.h"
#include "defaults.h"
#include "GfxMenu.h"
#include "MainMenu.h"
#include "IbsMenu.h"
#include "TrumaCombiMenu.h"
#include "SetupMenu.h"
#include "HelpMenu.h"

#include <Fonts/FreeMonoBold12pt7b.h>

//######################################
#include "HellaIbs.h"
HellaIbs hellaIbs;

//######################################
#include "LinDriver.h"

//######################################
// Menu item pointers:
MainMenu* mainMenu;
IbsMenu* ibsMenu;
TrumaCombiMenu* combiMenu;
SetupMenu* setupMenu;
HelpMenu* helpMenu;

GfxMenu::GfxMenu(uint8_t pinCs, uint8_t pinDc, uint8_t pinRst, uint8_t pinBacklight, uint8_t pinPwr) {
        this->pinChipSelect = pinCs;
        this->pinDataCommand = pinDc;
        this->pinReset = pinRst;
        this->pinBacklight = pinBacklight;
        this->pinPower = pinPwr;
}

GfxMenu::~GfxMenu(void) {
        // TODO: fix: "warning: deleting object of polymorphic class type 'Adafruit_ILI9341' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]"
        //delete adaIli9431;
}

void GfxMenu::setup(LinDriver* linDriver) {
        lastMenuCountUpdate = 0;
        lastMenuItemCount = 0;

        if (linDriver) {
                linDriver->begin();
                hellaIbs.setup(linDriver);
        }

        if (0 != pinPower) {
                Serial.println("Configuring TFT power pin...");
                pinMode(pinReset, OUTPUT);
                digitalWrite(pinReset, LOW);
                pinMode(pinPower, OUTPUT);
                tftPowerUp();
        }

        if (0 != pinBacklight) {
                Serial.println("Configuring TFT backlight pin...");
                pinMode(pinBacklight, OUTPUT);
                digitalWrite(pinBacklight, LOW);
        }

        if (!adaIli9431) {
                adaIli9431 = new Adafruit_ILI9341(pinChipSelect, pinDataCommand);         // Display library setup
        }
        if (adaIli9431) {
                Defaults.setup(adaIli9431);

                adaIli9431->setFont(Defaults.getFont());
                /*
                   Runs good at 78 MHz but not at 80 MHz.
                   Runs really stable @ 72 MHz but when it comes to write full screen (320x240), about 36 MHz seems to be the limit.
                 */
                adaIli9431->begin(32000000UL);
                adaIli9431->setRotation(3);

                changeLoopState(kGmlsPrepareBootLogo);

                //######################################
                // Initialize MenuItems:
                ibsMenu = new IbsMenu(adaIli9431, "Battery", &hellaIbs);
                combiMenu = new TrumaCombiMenu(adaIli9431, "Truma Heating");
                setupMenu = new SetupMenu(adaIli9431, "Setup");
                helpMenu = new HelpMenu(adaIli9431, "Help");
                mainMenu = new MainMenu(adaIli9431, "Main", ibsMenu);

                //######################################
                // Build up menu structure here:
                // TODO: Create more menu items and their structure here.
                mainMenu->setPrevMenuItem(0);
                mainMenu->setNextMenuItem(ibsMenu);
                ibsMenu->setPrevMenuItem(mainMenu);
                ibsMenu->setNextMenuItem(combiMenu);
                combiMenu->setPrevMenuItem(ibsMenu);
                combiMenu->setNextMenuItem(setupMenu);
                setupMenu->setPrevMenuItem(combiMenu);
                setupMenu->setNextMenuItem(helpMenu);
                helpMenu->setPrevMenuItem(setupMenu);
                helpMenu->setNextMenuItem(0);

                currentMenu = mainMenu;
        }
}


void GfxMenu::changeLoopState(GfxMenuLoopState newState) {
        loopState = newState;
        lastLoopStateChange = millis();
}

void GfxMenu::loop(void) {
        hellaIbs.loop();

        uint32_t durationSinceLastStateChange =  millis() - lastLoopStateChange;

        switch (loopState) {
        case kGmlsPrepareBootLogo:
                printBootLogo();
                digitalWrite(pinBacklight, HIGH);
                changeLoopState(kGmlsShowBootLogo);
                break;

        case kGmlsShowBootLogo:
                if (1234 /* msec */ < durationSinceLastStateChange) {
                        changeLoopState(kGmlsEnterMenu);
                }
                break;

        case kGmlsEnterMenu:
                digitalWrite(pinBacklight, LOW);
                if (currentMenu == mainMenu) {
                        printMenuScrollbar();
                }
                updateMenuCount();
                updateMenuScrollbar();
                currentMenu->printScreen();
                currentMenu->updateScreen();
                digitalWrite(pinBacklight, HIGH);
                changeLoopState(kGmlsUpdateMenu);
                break;

        case kGmlsUpdateMenu:
                if (1000 < millis() - lastMenuCountUpdate) { // every second...
                        // ...check for new devices
                        updateMenuCount();
                        lastMenuCountUpdate = millis();
                        updateMenuScrollbar();
                        if (!currentMenu->isVisible()) {
                                // Go Back to main menu if the current menu's device has gone.
                                currentMenu = mainMenu;
                                changeLoopState(kGmlsEnterMenu);
                        }
                }
                currentMenu->updateScreen();
                break;

        default:;
        }
}

void GfxMenu::printBootLogo(void) {
        adaIli9431->fillScreen(Defaults.getBgColor());
        adaIli9431->setTextColor(Defaults.getFgColor(), Defaults.getBgColor());
        adaIli9431->setFont(&FreeMonoBold12pt7b);

        String line1 = "RV Smart Control";
        String line2 = "by ChrisPHL";
        String line3 = "www.chpohl.de";
        int16_t x1, y1;
        uint16_t w, h;

        adaIli9431->getTextBounds(line1, 30, 0, &x1, &y1, &w, &h);
        adaIli9431->setCursor(DISPLAY_W / 2 - w / 2, DISPLAY_H / 3);
        adaIli9431->print(line1);

        adaIli9431->getTextBounds(line2, 30, 0, &x1, &y1, &w, &h);
        adaIli9431->setCursor(DISPLAY_W / 2 - w / 2, DISPLAY_H / 3 * 2);
        adaIli9431->print(line2);

        adaIli9431->getTextBounds(line3, 30, 0, &x1, &y1, &w, &h);
        adaIli9431->setCursor(DISPLAY_W / 2 - w / 2, DISPLAY_H / 3 * 2 + h);
        adaIli9431->print(line3);

        adaIli9431->setFont(Defaults.getFont());
}

void GfxMenu::printMenuScrollbar(void) {
        adaIli9431->drawRect(SCROLLBAR_X, SCROLLBAR_Y, SCROLLBAR_W, SCROLLBAR_H, Defaults.getFgColor());
}

void GfxMenu::updateMenuScrollbar(void){
        const uint16_t xCorrection = (SCROLLBAR_W - 2) % menuItemCount;
        const uint16_t handleW = (SCROLLBAR_W - 2) / menuItemCount + xCorrection / 2;
        const uint8_t currentMenuIndex = getCurrentMenuIndex();

        // Black space _before_ handle:
        adaIli9431->fillRect(SCROLLBAR_X + 1, SCROLLBAR_Y + 1, handleW * currentMenuIndex, SCROLLBAR_H - 2, Defaults.getBgColor());
        // White handle:
        uint16_t startHandleX = 1;
        if (currentMenuIndex == menuItemCount - 1) { // printing scrollbar at the very last position:
                startHandleX = SCROLLBAR_X + 1 + SCROLLBAR_W - 2 - handleW;
        } else {
                startHandleX = SCROLLBAR_X + 1 + currentMenuIndex * handleW;
        }
        adaIli9431->fillRect(startHandleX, SCROLLBAR_Y + 1, handleW, SCROLLBAR_H - 2, Defaults.getFgColor());
        if (currentMenu->isMenuEntered()) {
                adaIli9431->fillRect(startHandleX + 1, SCROLLBAR_Y + 1, handleW - 2, SCROLLBAR_H - 2, Defaults.getIndicatorColor());
        }
        // Black space _after_ handle:
        if (currentMenuIndex != menuItemCount - 1) { // Last item, not trailing black portion.
                const uint8_t trailingStartX = startHandleX + handleW;
                adaIli9431->fillRect(trailingStartX, SCROLLBAR_Y + 1, SCROLLBAR_W - 2 - trailingStartX + 1, SCROLLBAR_H - 2, Defaults.getBgColor());
        }
}

void GfxMenu::inputLeft(uint8_t count) {
        if (kGmlsShowBootLogo >= loopState) {
                currentMenu = mainMenu;
                changeLoopState(kGmlsEnterMenu);
        } else {
                while (0 < count--) {
                        if (currentMenu->isMenuEntered()) {
                                currentMenu->inputLeft();
                        } else {
                                do {
                                        if (0 != currentMenu->getPrevMenuItem()) {
                                                currentMenu = currentMenu->getPrevMenuItem();
                                                changeLoopState(kGmlsEnterMenu);
                                        }
                                } while (!currentMenu->isVisible());
                        }
                }
        }
}

void GfxMenu::inputRight(uint8_t count) {
        if (kGmlsShowBootLogo >= loopState) {
                currentMenu = mainMenu;
                changeLoopState(kGmlsEnterMenu);
        } else {
                while (0 < count--) {
                        if (currentMenu->isMenuEntered()) {
                                currentMenu->inputRight();
                        } else {
                                do {
                                        if (0 != currentMenu->getNextMenuItem()) {
                                                currentMenu = currentMenu->getNextMenuItem();
                                                changeLoopState(kGmlsEnterMenu);
                                        }
                                } while (!currentMenu->isVisible());
                        }
                }
        }
}

void GfxMenu::inputPush(uint8_t count) {
        if (kGmlsShowBootLogo >= loopState) {
                currentMenu = mainMenu;
                changeLoopState(kGmlsEnterMenu);
        } else {
                if (!currentMenu->isMenuEntered()) {
                        currentMenu->onEnterMenu();
                        updateMenuScrollbar();
                } else {
                        currentMenu->inputPush();
                }
        }
}

void GfxMenu::inputLongPress(uint8_t count) {
        if (kGmlsShowBootLogo >= loopState) {
                currentMenu = mainMenu;
                changeLoopState(kGmlsEnterMenu);
        } else {
                if (currentMenu->isMenuEntered()) {
                        currentMenu->onLeaveMenu();
                        updateMenuScrollbar();
                }
                if (5 <= count) {
                        Serial.println("Very long button press detected. Forcing system reboot.");
                        ESP.restart();
                }
        }
}

void GfxMenu::timeout(void) {
        if (currentMenu != mainMenu) { // Avoids printScreen() flickering.
                currentMenu = mainMenu;
                changeLoopState(kGmlsEnterMenu);
        }
}

void GfxMenu::tftPowerUp(void) {
        if (0 != pinPower) {
                digitalWrite(pinPower, HIGH);
        }
        delay(10);
        digitalWrite(pinReset, HIGH);
}

void GfxMenu::tftPowerDown(void) {
        digitalWrite(pinBacklight, LOW);
        digitalWrite(pinReset, LOW);
        tftPowerUp();
        if (0 != pinPower) {
                digitalWrite(pinPower, LOW);
        }
}

/**
 * This returns the index if the currently active menu.
 *
 * @return Menu index from 0 to <count_of_menuitems> - 1
 */
uint8_t GfxMenu::getCurrentMenuIndex(void) {
        MenuItem* tmpMenu = mainMenu;
        uint8_t retVal = 0;
        while (true) {
                if (currentMenu == tmpMenu) {
                        break;
                }
                if (tmpMenu->isVisible()) {
                        ++retVal;
                }
                if (0 != tmpMenu->getNextMenuItem()) {
                        tmpMenu = tmpMenu->getNextMenuItem();
                }
        }
        return retVal;
}

void GfxMenu::updateMenuCount(void) {
        MenuItem* tmpMenu = mainMenu;
        menuItemCount = 1;
        while (0 != tmpMenu->getNextMenuItem()) {
                tmpMenu = tmpMenu->getNextMenuItem();
                if (tmpMenu->isVisible()) {
                        ++menuItemCount;
                }
        }
        if (lastMenuItemCount != menuItemCount) {
                lastMenuItemCount = menuItemCount;
        }
}
