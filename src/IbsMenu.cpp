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

#include "IbsMenu.h"
#include "defaults.h"

static const uint16_t batteryX = 10; // upper left X
static uint16_t batteryY = 25; // upper left Y
static const uint16_t batteryWidth = 80;
static const uint16_t batteryHeight = 150;
static const uint16_t batteryPoleW = 16; // battery pole hight
static const uint16_t batteryPoleH = 8; // battery pole width

static const uint16_t ampereMeterX = 235;
static uint16_t ampereMeterY = 20;
static const uint16_t ampereMeterW = 20;
static const uint16_t ampereMeterH = 150; // Choose an odd value!
static const uint16_t ampMeterCenterY = ampereMeterH / 2 + 1;
static const uint16_t ampereMeterMarkerW = 12;
static const uint16_t ampereMeterFactorX = ampereMeterX + ampereMeterW;
static const uint16_t ampereMeterNeedleH = 6;

static const uint16_t statsX = 105;
static uint16_t statsY = 30;

static uint64_t lastDisplayErrorTime = millis();
static uint16_t errorColor[2] = {Defaults.getBgColor(), Defaults.getErrorColor()};
static uint8_t errorColorIndex = 0;

static uint8_t lastSoc;
static uint8_t lastCalibrated;
static float lastBatteryCurrent;
static int16_t lastIx;
static float lastBatteryVoltage;
static float lastAvailableCapacity;
static float lastDischargeableCapacity;
static uint8_t lastNominalCapacity;
static float lastTemperature;
static uint8_t lastSoh;
static IbsBatteryType lastBatteryType;


static uint16_t nominalCapacitySetupValue;
static uint8_t battTypeSelectionIndex;
static const uint8_t battTypesCount = 3;
static const IbsBatteryType battTypes[battTypesCount] = {kBatteryTypeStd, kBatteryTypeGel, kBatteryTypeAgm};


IbsMenu::IbsMenu(Adafruit_ILI9341* adaIli9431, HellaIbs* ibs)
        : MenuItem(adaIli9431), MenuBatteryStatus() {
        this->adaIli9431 = adaIli9431;
        this->ibs = ibs;
        batteryY = Defaults.getFontH() * 3;
        ampereMeterY = Defaults.getFontH() * 3;
        statsY = Defaults.getFontH() * 4;
}

IbsMenu::IbsMenu(Adafruit_ILI9341* adaIli9431, String headline, HellaIbs* ibs)
        : MenuItem(adaIli9431, headline), MenuBatteryStatus() {
        this->adaIli9431 = adaIli9431;
        this->ibs = ibs;
        batteryY = Defaults.getFontH() * 3;
        ampereMeterY = Defaults.getFontH() * 3;
        statsY = Defaults.getFontH() * 4;
}

void IbsMenu::printScreenImplementation(void) {
        commonPrintScreen();
        ibs->setHighSpeedCommunication(true);

        // Battery indicator:
        drawBatteryIndicator(batteryX, batteryY, batteryWidth, batteryHeight, batteryPoleH, batteryPoleW);

        h = Defaults.getFontH();
        w = Defaults.getFontCharW();

        // Battery stats:
        adaIli9431->setCursor(statsX, statsY + 0 * h); adaIli9431->print("U   :"); // Battery voltage
        adaIli9431->setCursor(statsX, statsY + 1 * h); adaIli9431->print("Cavl:"); // Available capacity
        adaIli9431->setCursor(statsX, statsY + 2 * h); adaIli9431->print("Cdis:"); // Dischargable capacity
        adaIli9431->setCursor(statsX, statsY + 3 * h); adaIli9431->print("Cnom:"); // Nominal capacity

        adaIli9431->setCursor(statsX, statsY + 9 * h / 2); adaIli9431->print("T   :"); // Battery temperature

        adaIli9431->setCursor(statsX, statsY + 6 * h); adaIli9431->print("Type:"); // Battery type: Std|AGM|Gel
        adaIli9431->setCursor(statsX, statsY + 7 * h); adaIli9431->print("SOH :"); // State of health

        adaIli9431->setFont(Defaults.getMiniFont());
        int16_t x1, y1;
        uint16_t w1, h1;
        adaIli9431->getTextBounds(ibs->getName(), 0, 0, &x1, &y1, &w1, &h1);
        adaIli9431->setCursor(DISPLAY_W / 2 - w1 / 2, statsY + 9 * h);
        adaIli9431->print(ibs->getName()); // IBS name
        adaIli9431->setFont(Defaults.getFont());

        // Amperemeter:
        uint16_t x = ampereMeterX;
        uint16_t y = ampereMeterY;
        adaIli9431->fillRect(x, y - ampereMeterNeedleH / 2, ampereMeterW, ampereMeterH + (ampereMeterNeedleH / 2) * 2, Defaults.getFgColor());
        adaIli9431->drawFastHLine(x + ampereMeterW, y, ampereMeterMarkerW, Defaults.getFgColor()); // + max line
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY - ampMeterCenterY * 3 / 4, ampereMeterMarkerW / 2, Defaults.getFgColor());
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY - ampMeterCenterY / 2, ampereMeterMarkerW, Defaults.getFgColor()); // + middle line
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY - ampMeterCenterY * 1 / 4, ampereMeterMarkerW / 2, Defaults.getFgColor());
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY, ampereMeterMarkerW, Defaults.getFgColor()); // zero line
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY + ampMeterCenterY * 1 / 4, ampereMeterMarkerW / 2, Defaults.getFgColor());
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY + ampMeterCenterY / 2, ampereMeterMarkerW, Defaults.getFgColor()); // - middle line
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampMeterCenterY + ampMeterCenterY * 3 / 4, ampereMeterMarkerW / 2, Defaults.getFgColor());
        adaIli9431->drawFastHLine(x + ampereMeterW, y + ampereMeterH - 1, ampereMeterMarkerW, Defaults.getFgColor()); // - max line

        adaIli9431->setTextColor(Defaults.getFgColor(), ILI9341_BLACK);

        adaIli9431->setCursor(x + ampereMeterW + ampereMeterMarkerW * 1.5, y + Defaults.getFontY() / 2);
        adaIli9431->print("+2");
        adaIli9431->setCursor(x + ampereMeterW + ampereMeterMarkerW * 1.5, y + ampMeterCenterY - ampMeterCenterY / 2 + Defaults.getFontY() / 2);
        adaIli9431->print("+1");
        adaIli9431->setCursor(x + ampereMeterW + ampereMeterMarkerW * 1.5, y + ampMeterCenterY + Defaults.getFontY() / 2);
        adaIli9431->print(" 0");
        adaIli9431->setCursor(x + ampereMeterW + ampereMeterMarkerW * 1.5, y + ampMeterCenterY + ampMeterCenterY / 2 + Defaults.getFontY() / 2);
        adaIli9431->print("-1");
        adaIli9431->setCursor(x + ampereMeterW + ampereMeterMarkerW * 1.5, y + ampereMeterH + Defaults.getFontY() / 2);
        adaIli9431->print("-2");

        lastSoc = 199;
        lastCalibrated = 99;
        lastBatteryCurrent = -999.9;
        lastIx = 0xff;
        lastBatteryVoltage = -999.9;
        lastAvailableCapacity = -999.9;
        lastDischargeableCapacity = -999.9;
        lastNominalCapacity = 0;
        lastTemperature = -99.9;
        lastSoh = 0;
        lastBatteryType = (IbsBatteryType) 0x00;

        nominalCapacitySetupValue = 0xffff;
}

void IbsMenu::updateScreenImplementation(void) {
        updateBatteryIndicator(batteryX, batteryY, batteryWidth, batteryHeight);

        h = Defaults.getFontH();
        w = Defaults.getFontCharW();
        x1 = statsX + 5 * w + 10;
        uint16_t fgColor = Defaults.getFgColor();
        uint16_t bgColor = ILI9341_BLACK;

        float voltage = ibs->getBatteryVoltage();
        if (lastBatteryVoltage != voltage) {
                snprintf(string, sizeof string, "%2d.%1dV", (uint8_t)voltage, (uint8_t)(voltage * 10) % 10);
                updateDisplayText(string, x1, statsY + 0 * h, fgColor, bgColor);
                lastBatteryVoltage = voltage;
        }

        float availableCapacity = ibs->getAvailableCapacity();
        if (lastAvailableCapacity != availableCapacity) {
                snprintf(string, sizeof string, "%3dAh", (uint8_t)availableCapacity);
                updateDisplayText(string, x1, statsY + 1 * h, fgColor, bgColor); // Available capacity
                lastAvailableCapacity = availableCapacity;
        }

        float dischargeableCapacity = ibs->getDischargeableCapacity();
        if (lastDischargeableCapacity != dischargeableCapacity) {
                snprintf(string, sizeof string, "%3dAh", (uint8_t)dischargeableCapacity);
                updateDisplayText(string, x1, statsY + 2 * h, fgColor, bgColor); // Dischargable capacity
                lastDischargeableCapacity = dischargeableCapacity;
        }

        int16_t nominalCapacity;
        if (menuEntered) {
                nominalCapacity = nominalCapacitySetupValue;
        } else {
                nominalCapacity = ibs->getNominalCapacity();
        }
        if (lastNominalCapacity != nominalCapacity) {
                updateNominalCapacityStat(x1, statsY + 3 * h, nominalCapacity, menuEntered && selectionFocus.at(0), itemSelected);
                lastNominalCapacity = nominalCapacity;
        }

        float temperature = ibs->getTemperature();
        if (lastTemperature != temperature) {
                snprintf(string, sizeof string, "%+2d C", (int8_t)temperature);
                uint16_t tempY = statsY + 9 * h / 2;
                updateDisplayText(string, x1, tempY, fgColor, bgColor); // Battery temperature
                adaIli9431->drawCircle(x1 + 4 * w, tempY - Defaults.getFontH() / 2, 2, fgColor); // degree
                adaIli9431->drawCircle(x1 + 4 * w, tempY - Defaults.getFontH() / 2, 3, fgColor); // bold
                lastTemperature = temperature;
        }

        IbsBatteryType batteryType;
        if (menuEntered) {
                batteryType = battTypes[battTypeSelectionIndex];
        } else {
                batteryType = ibs->getBatteryType();
        }
        if (lastBatteryType != batteryType) {
                updateBatteryTypeStat(x1, statsY + 6 * h, batteryType, menuEntered && selectionFocus.at(1), itemSelected);
                lastBatteryType = batteryType;
        }

        uint8_t soh = ibs->getSoh();
        if (lastSoh != soh) {
                snprintf(string, sizeof string, " %3d%%", soh);
                updateDisplayText(string, x1, statsY + 7 * h, fgColor, bgColor); // State of health
                lastSoh = soh;
        }

        float current = ibs->getBatteryCurrent();
        if (lastBatteryCurrent != current) {
                boolean currentOutOfScope = false;
                boolean charging = true;
                int16_t ix;

                int16_t x1, y1;
                uint16_t w, h;
                adaIli9431->getTextBounds("x100", 0, 0, &x1, &y1, &w, &h);
                uint16_t handleColor = ILI9341_BLUE;

                if (current < 0) {
                        current = -current;
                        charging = false;
                }

                uint8_t charCount = 0;
                // Ugly workaround for arduino printf() does not work with %f formatter:
                if (current < 10) {
                        charCount = snprintf(string, sizeof string, "%c%1d.%02dA", charging ? '+' : '-', (uint8_t)current, (uint8_t)(current * 100) % 100);
                } else if (10 <= current && current < 100) {
                        charCount = snprintf(string, sizeof string, "%c%2d.%1dA", charging ? '+' : '-', (uint8_t)current, (uint8_t)(current * 10) % 10);
                } else if (100 <= current) {
                        charCount = snprintf(string, sizeof string, " %c%3dA", charging ? '+' : '-', (int8_t)current);
                }
                string[charCount] = '\0';

                if (current > 200) {
                        current = 200;
                        currentOutOfScope = true;
                }

                String ampFactorString;
                if (current <= 2) {
                        ampFactorString = "    ";
                        ix = uint8_t(ampMeterCenterY * current / 2.0);
                } else if ((current > 2) && (current <= 20)) {
                        ampFactorString = " x10";
                        ix = uint8_t(ampMeterCenterY * current / 20.0);
                } else /*if (current > 20)*/ {
                        ampFactorString = "x100";
                        ix = uint8_t(ampMeterCenterY * current / 200.0);
                }
                updateDisplayText(ampFactorString.c_str(), ampereMeterFactorX, ampereMeterY - Defaults.getFontH(), fgColor, bgColor);

                if (!charging) {
                        ix = -ix;
                        handleColor = ILI9341_RED;
                }
                if (currentOutOfScope) {
                        handleColor = ILI9341_YELLOW;
                }
                if (lastIx != ix) {
                        if (0xff != lastIx) {
                                // Clear last needle display if lastIx is valid:
                                adaIli9431->fillRect(ampereMeterX + 1, ampereMeterY + ampMeterCenterY - ampereMeterNeedleH / 2 - lastIx, ampereMeterW - 2, ampereMeterNeedleH, Defaults.getFgColor());
                        }
                        adaIli9431->fillRect(ampereMeterX + 1, ampereMeterY + ampMeterCenterY - ampereMeterNeedleH / 2 - ix, ampereMeterW - 2, ampereMeterNeedleH, handleColor);
                        lastIx = ix;
                }
                updateDisplayText(string, ampereMeterX, ampereMeterY + ampereMeterH + Defaults.getFontY() * 2, fgColor, bgColor);

                lastBatteryCurrent = current;
        }
}

void IbsMenu::onEnterButtonImplementation(void) {
        if (ibs->isAvailable()) {
                selectionIndex = 0;
                updateSelectionFocus();

                itemSelected = false;
                menuEntered = true;
                lastNominalCapacity = 0;
                lastBatteryType = (IbsBatteryType) 0x00;
                nominalCapacitySetupValue = ibs->getNominalCapacity();
                IbsBatteryType battType = ibs->getBatteryType();
                for (uint8_t i = 0; i<battTypesCount; ++i) {
                        if (battTypes[i] == battType) {
                                battTypeSelectionIndex = i;
                                break;
                        }
                }
        }
}

void IbsMenu::onLeaveButtonImplementation(void) {
        menuEntered = false;
        lastNominalCapacity = 0;
        lastBatteryType = (IbsBatteryType) 0x00;
        ibs->configure(nominalCapacitySetupValue, battTypes[battTypeSelectionIndex]);
}

void IbsMenu::inputLeft(void) {
        if (itemSelected) {
                if (0 == selectionIndex) { // Nominal Capacity
                        if (0 < nominalCapacitySetupValue) {
                                --nominalCapacitySetupValue;
                        }
                } else if (1 == selectionIndex) { // Battery Type
                        if (0 < battTypeSelectionIndex) {
                                --battTypeSelectionIndex;
                        }
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

void IbsMenu::inputRight(void) {
        if (itemSelected) {
                if (0 == selectionIndex) { // Nominal Capacity
                        if (249 > nominalCapacitySetupValue) {
                                ++nominalCapacitySetupValue;
                        }
                } else if (1 == selectionIndex) { // Battery Type
                        if ((battTypesCount -1) > battTypeSelectionIndex) {
                                ++battTypeSelectionIndex;
                        }
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

void IbsMenu::inputPush(void) {
        if (itemSelected) {
                itemSelected = false;
        } else {
                itemSelected = true;
        }
        updateSelectionFocus();
}

void IbsMenu::updateSelectionFocus(void) {
        // 1.: Nominal Capacity, 2.: Battery Type
        selectionFocus.clear();
        selectionFocus.assign(2, false);
        selectionFocus.at(selectionIndex) = true;

        lastNominalCapacity = 0;
        lastBatteryType = (IbsBatteryType) 0x00;
}

void IbsMenu::drawBatteryIndicator(uint16_t x, uint16_t y, uint8_t w, uint8_t h, uint8_t poleH, uint8_t poleW) {
        uint16_t xPoleOffset = (w / 2 - poleW) / 2;
        adaIli9431->fillRect(x + xPoleOffset, y - poleH, poleW, poleH, Defaults.getFgColor());
        adaIli9431->fillRect(x + w - xPoleOffset - poleW, y - poleH, poleW, poleH, Defaults.getFgColor());
        adaIli9431->drawRect(x, y, w, h, Defaults.getFgColor());
        lastSoc = 0xff;
}

void IbsMenu::updateBatteryIndicator(uint16_t batteryX, uint16_t batteryY, uint16_t batteryWidth, uint16_t batteryHeight) {
        uint8_t soc = ibs->getSoc();
        uint8_t calibrated = ibs->isCalibrated();
        if (lastSoc != soc || lastCalibrated != calibrated) {
                uint16_t color = Defaults.getFgColor();
                if (80 < soc) color = ILI9341_GREEN;
                else if (80 >= soc && soc > 20) {
                        // Calculate color gradient between 80% and 20% from green to red.
                        uint8_t gradient = (soc - 20) * 100 / 60;
                        color = getColorGradient(ILI9341_RED, ILI9341_GREEN, gradient);
                }
                else if (20 >= soc) color = ILI9341_RED;
                uint16_t socHeight = soc * (batteryHeight - 4) /  100;

                adaIli9431->fillRect(batteryX + 2, batteryY + 2, batteryWidth - 4, batteryHeight - 4 - socHeight, Defaults.getBgColor());
                adaIli9431->fillRect(batteryX + 2, batteryY + 2 + (batteryHeight - 4 - socHeight), batteryWidth - 4, socHeight, color);

                if (calibrated) {
                        color = Defaults.getFgColor();
                } else {
                        color = Defaults.getErrorColor();
                }
                if (0 != soc) {
                        snprintf(string, sizeof string, "%3d%%", ibs->getSoc());
                } else {
                        snprintf(string, sizeof string, "n/a ");
                }
                updateDisplayText(string, batteryX + batteryWidth / 2 - Defaults.getFontCharW() * 2, batteryY + batteryHeight + Defaults.getFontY() * 2, color, Defaults.getBgColor());

                lastSoc = soc;
                lastCalibrated = calibrated;
        } else if (ibs->isError()) {
                if (millis() - lastDisplayErrorTime >= 1000) {
                        lastDisplayErrorTime = millis();
                        if (0 == errorColorIndex) {
                                errorColorIndex = 1;
                        } else {
                                errorColorIndex = 0;
                        }
                        adaIli9431->fillRect(batteryX + 2, batteryY + 2, batteryWidth - 4, batteryHeight - 4, errorColor[errorColorIndex]);
                }
        }
}

void IbsMenu::updateNominalCapacityStat(uint16_t x, uint16_t y, int16_t capacity, boolean highlighted, boolean selected) {
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
        snprintf(string, sizeof string, "%3dAh", capacity);
        updateDisplayText(string, x, y, fgColor, bgColor); // Nominal capacity
}

void IbsMenu::updateBatteryTypeStat(uint16_t x, uint16_t y, IbsBatteryType batteryType, boolean highlighted, boolean selected) {
        switch (batteryType) {
        case kBatteryTypeStd:
                snprintf(string, sizeof string, "  Std");
                break;
        case kBatteryTypeGel:
                snprintf(string, sizeof string, "  Gel");
                break;
        case kBatteryTypeAgm:
                snprintf(string, sizeof string, "  AGM");
                break;
        default:
                snprintf(string, sizeof string, "  ---");
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
        updateDisplayText(string, x, y, fgColor, bgColor); // Battery type
}
