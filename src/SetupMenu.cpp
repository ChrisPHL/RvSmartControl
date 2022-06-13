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

#include "SetupMenu.h"
#include "Persistence.h"
#include "WiFiController.h"


static uint8_t lastStartWifiConfig = 0xff;
static uint8_t lastWifiOnOff = 0xff;
static boolean wifiSsidChanged = true;
static uint8_t lastFactoryReset = 0xff;
static WiFiControllerLoopState lastWifiState = kWclsIdle;

static uint8_t startWifiConfig = 0;
static boolean runningWifiConfig = false;
static uint8_t factoryReset = 0;

static uint16_t wifiOnOffX = 0;
static uint16_t wifiOnOffY = 0;
static uint16_t wifiStartWifiConfigX = 0;
static uint16_t wifiStartWifiConfigY = 0;
static uint16_t wifiSsidX = 0;
static uint16_t wifiSsidY = 0;
static uint16_t factoryResetX = 0;
static uint16_t factoryResetY = 0;

const char characters[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'p', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        '&', '\'', '(', '"', '!', ')', '-', '@', '#', '^', '%', ',', '?', ';', '.', ':', '/', '=', '+', '!', '|', '_', ' ', '$'
};

void SetupMenu::printScreenImplementation(void) {
        commonPrintScreen();

        adaIli9431->setCursor(0, 3 * Defaults.getFontH());

        adaIli9431->print("WiFi: ");
        wifiOnOffX = adaIli9431->getCursorX(); wifiOnOffY = adaIli9431->getCursorY();
        adaIli9431->println();
        adaIli9431->print("WiFi Smart Config: ");
        wifiStartWifiConfigX = adaIli9431->getCursorX(); wifiStartWifiConfigY = adaIli9431->getCursorY();
        adaIli9431->println();
        adaIli9431->println("WiFi SSID: "); // SSID starts in new line since length exceeds display width.
        wifiSsidX = adaIli9431->getCursorX(); wifiSsidY = adaIli9431->getCursorY();
        adaIli9431->println();
        adaIli9431->println("Paired Bluetooth devices:");
        adaIli9431->print("  #1: ");
        adaIli9431->println(Persistence::getInstance().readSlot(kPSlotBluetoothPair1));
        adaIli9431->print("  #2: ");
        adaIli9431->println(Persistence::getInstance().readSlot(kPSlotBluetoothPair2));
        adaIli9431->print("  #3: ");
        adaIli9431->println(Persistence::getInstance().readSlot(kPSlotBluetoothPair3));
        adaIli9431->print("  #4: ");
        adaIli9431->println(Persistence::getInstance().readSlot(kPSlotBluetoothPair4));
        adaIli9431->println();
        adaIli9431->print("Factory reset: ");
        factoryResetX = adaIli9431->getCursorX(); factoryResetY = adaIli9431->getCursorY();

        wifiOnOff = Persistence::getInstance().readSlotBoolean(kPSlotWiFiOnOff);
        startWifiConfig = 0;
        factoryReset = 0;

        forceUpdateDisplay();
}

void SetupMenu::updateScreenImplementation(void) {
        if (lastWifiOnOff != wifiOnOff) {
                updateWifiOnOff(wifiOnOffX, wifiOnOffY, wifiOnOff, menuEntered && selectionFocus.at(0), itemSelected);
                lastWifiOnOff = wifiOnOff;
        }

        if (lastStartWifiConfig != startWifiConfig) {
                updateWifiSmartConfig(wifiStartWifiConfigX, wifiStartWifiConfigY, startWifiConfig, menuEntered && selectionFocus.at(1), itemSelected);
                lastStartWifiConfig = startWifiConfig;
        }

        WiFiControllerLoopState wifiState = WiFiController::getInstance().getState();
        if (lastWifiState != wifiState) {
                wifiSsidChanged = true;
                lastWifiState = wifiState;
        }
        if (wifiSsidChanged) {
                adaIli9431->setCursor(wifiSsidX, wifiSsidY);
                if (runningWifiConfig) {
                        snprintf(string, sizeof string, "%-28s", "Now run mobile App.");
                        updateDisplayText(string, wifiSsidX, wifiSsidY, Defaults.getFgColor(), Defaults.getBgColor());
                } else if (1 == startWifiConfig) {
                        snprintf(string, sizeof string, "%-28s", "Press to (re-) configure.");
                        updateDisplayText(string, wifiSsidX, wifiSsidY, Defaults.getFgColor(), Defaults.getBgColor());
                } else if (kWclsWifiUpAndRunning == WiFiController::getInstance().getState()) {
                        snprintf(string, sizeof string, "%-28s", Persistence ::getInstance().readSlot(kPSlotWiFiSsid).c_str());
                        updateDisplayText(string, wifiSsidX, wifiSsidY, Defaults.getFgColor(), Defaults.getBgColor());
                } else {
                        snprintf(string, sizeof string, "%-28s", "unconfigured / disconnected");
                        updateDisplayText(string, wifiSsidX, wifiSsidY, Defaults.getFgColor(), Defaults.getBgColor());
                }
                wifiSsidChanged = false;
        }

        if (1 == startWifiConfig) {
                if (!runningWifiConfig) {
                        if (kWclsSetupSmartConfig == WiFiController::getInstance().getState()) {
                                Serial.println("Configuration process started.");
                                runningWifiConfig = true;
                                wifiSsidChanged = true;
                        }
                } else {
                        if (kWclsSetupSmartConfig != WiFiController::getInstance().getState()) {
                                Serial.println("Configuration process is over");
                                wifiOnOff = 1;
                                Persistence::getInstance().writeSlotBoolean(kPSlotWiFiOnOff, wifiOnOff);
                                runningWifiConfig = false; // Just cleanup the value.
                                startWifiConfig = 0;
                                wifiSsidChanged = true;
                        }
                }
        }

        if (lastFactoryReset != factoryReset) {
                updateFactoryReset(factoryResetX, factoryResetY, factoryReset, menuEntered && selectionFocus.at(2), itemSelected);
                lastFactoryReset = factoryReset;
        }
}

void SetupMenu::updateWifiOnOff(uint16_t x, uint16_t y, uint8_t onOff, boolean highlighted, boolean selected) {
        switch (onOff) {
        case 1:
                snprintf(string, sizeof string, " on");
                break;
        case 0:
        default:
                snprintf(string, sizeof string, "off");
                break;
        }
        uint16_t fgColor = Defaults.getFgColor();
        uint16_t bgColor = Defaults.getBgColor();
        if (highlighted) {
                fgColor = Defaults.getFgHighlightColor();
                if (!selected) {
                        bgColor = Defaults.getBgHighlightColor();
                } else {
                        bgColor = Defaults.getBgHlSelectedColor();
                }
        }
        updateDisplayText(string, x, y, fgColor, bgColor); // wifiOnOff
}

void SetupMenu::updateWifiSmartConfig(uint16_t x, uint16_t y, uint8_t start, boolean highlighted, boolean selected) {
        if (1 == start) {
                snprintf(string, sizeof string, "yes");
        } else {
                snprintf(string, sizeof string, "---");
        }

        uint16_t fgColor = Defaults.getFgColor();
        uint16_t bgColor = Defaults.getBgColor();
        if (highlighted) {
                fgColor = Defaults.getFgHighlightColor();
                if (!selected) {
                        bgColor = Defaults.getBgHighlightColor();
                } else {
                        bgColor = Defaults.getBgHlSelectedColor();
                }
        }
        updateDisplayText(string, x, y, fgColor, bgColor); // startWifiConfig
}

void SetupMenu::updateFactoryReset(uint16_t x, uint16_t y, uint8_t reset, boolean highlighted, boolean selected) {
        if (1 == reset) {
                snprintf(string, sizeof string, "yes");
        } else {
                snprintf(string, sizeof string, "---");
        }
        uint16_t fgColor = Defaults.getFgColor();
        uint16_t bgColor = Defaults.getBgColor();
        if (highlighted) {
                fgColor = Defaults.getFgHighlightColor();
                if (!selected) {
                        bgColor = Defaults.getBgHighlightColor();
                } else {
                        bgColor = Defaults.getBgHlSelectedColor();
                }
        }
        updateDisplayText(string, x, y, fgColor, bgColor); // factory reset
}

void SetupMenu::onEnterButtonImplementation(void) {
        selectionIndex = 0;
        updateSelectionFocus();

        itemSelected = false;
        menuEntered = true;
}

void SetupMenu::onLeaveButtonImplementation(void) {
        menuEntered = false;
        if (runningWifiConfig) {
                WiFiController::getInstance().abortWifiConfig();
                runningWifiConfig = false;
                forceUpdateDisplay();
        }
        if (1 == factoryReset) {
                Serial.println("Performing device factory reset.");
                Persistence::getInstance().eraseEeprom();
                ESP.restart();
        }
        forceUpdateDisplay();
}

void SetupMenu::inputLeft(void) {
        if (itemSelected) {
                if (0 == selectionIndex) { // WiFi on/off
                        wifiOnOff = 0; // off
                } else if (1 == selectionIndex) { // WiFi Smart Config
                        startWifiConfig = 0; // ---
                } else if (2 == selectionIndex) { // Factory Reset
                        factoryReset = 0; // ---
                }
        } else {
                if (0 < selectionIndex) {
                        --selectionIndex;
                } else {
                        selectionIndex = selectionFocus.size() - 1;
                }
        }
        updateSelectionFocus();
}

void SetupMenu::inputRight(void) {
        if (itemSelected) {
                if (0 == selectionIndex) { // WiFi on/off
                        wifiOnOff = 1; // on
                } else if (1 == selectionIndex) { // WiFi Smart Config
                        startWifiConfig = 1; // yes
                } else if (2 == selectionIndex) { // Factory Reset
                        factoryReset = 1; // yes
                }
        } else {
                if ((selectionFocus.size() - 1) > selectionIndex) {
                        ++selectionIndex;
                } else {
                        selectionIndex = 0;
                }
        }
        updateSelectionFocus();
}

void SetupMenu::inputPush(void) {
        if (itemSelected) {
                itemSelected = false;
                Persistence::getInstance().writeSlotBoolean(kPSlotWiFiOnOff, wifiOnOff);
                if (0 == wifiOnOff) {
                        WiFiController::getInstance().start(); // Force WiFi going down if it was enabled before.
                }
                if (1 == startWifiConfig) {
                        WiFiController::getInstance().setWifiConfigEnable();
                        runningWifiConfig = false;
                } else {
                        WiFiController::getInstance().abortWifiConfig();
                        startWifiConfig = 0; // ---
                        runningWifiConfig = false;
                        forceUpdateDisplay();
                }
        } else {
                itemSelected = true;
        }
        updateSelectionFocus();
}

void SetupMenu::updateSelectionFocus(void) {
        // 1.: WiFi on/off, 2.: WiFi SSID, 3.: Factory Reset
        selectionFocus.clear();
        selectionFocus.assign(3, false);
        selectionFocus.at(selectionIndex) = true;
        forceUpdateDisplay();
}

void SetupMenu::forceUpdateDisplay(void) {
        lastStartWifiConfig = 0xff;
        lastWifiOnOff = 0xff;
        wifiSsidChanged = true;
        lastFactoryReset = 0xff;
        lastWifiState = kWclsIdle;
}
