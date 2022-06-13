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

#ifndef DEFAULTS_H_
#define DEFAULTS_H_

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Org_01.h>


// Using 2.8" LCD display with touch controller:
// http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807
static const uint16_t DISPLAY_W = 320;
static const uint16_t DISPLAY_H = 240;

static const uint16_t SCROLLBAR_X = 0;
static const uint16_t SCROLLBAR_Y = 235;
static const uint16_t SCROLLBAR_W = 319;
static const uint16_t SCROLLBAR_H = 5;

class DefaultsClass {
private:
  bool font_height_set;
  uint8_t font_h;
  uint8_t font_y;
  uint8_t font_char_w;

public:
  DefaultsClass(void);

  void setup(Adafruit_ILI9341* gfxContext);

  /**
   * Returns the default font's height.
   */
  inline uint8_t getFontH(void) {
          return font_h;
  }

  /**
   * Returns the default font's height from ground line.
   */
  inline uint8_t getFontY(void) {
          return font_y;
  }

  /**
   * Returns the default font's height from ground line.
   */
  inline uint8_t getFontCharW(void) {
          return font_char_w;
  }

  /**
   * Returns the default font for the current system.
   */
  inline const GFXfont* getFont(void) {
          return &FreeMonoBold9pt7b;
  }

  /**
   * Returns the default font for the current system.
   */
  inline const GFXfont* getMiniFont(void) {
          return &Org_01;
  }

  /**
   * Returns the default gfx menu foreground color.
   */
  inline const uint16_t getFgColor(void) {
          return ILI9341_WHITE;
  }

  /**
   * Returns the default gfx menu background color.
   */
  inline const uint16_t getBgColor(void) {
          return ILI9341_BLACK;
  }

  /**
   * Returns the default gfx menu color for error display.
   */
  inline const uint16_t getErrorColor(void) {
          return ILI9341_RED;
  }

  /**
   * Returns the default gfx menu foreground color for highligted items.
   */
  inline const uint16_t getFgHighlightColor(void) {
          return ILI9341_BLACK;
  }

  /**
   * Returns the default gfx menu background color for highligted items.
   */
  inline const uint16_t getBgHighlightColor(void) {
          return ILI9341_WHITE;
  }

  /**
   * Returns the default gfx menu background color for highligted items.
   */
  inline const uint16_t getBgHlSelectedColor(void) {
          return ILI9341_ORANGE;
  }

  /**
   * Returns the default gfx menu background color for highligted items.
   */
  inline const uint16_t getIndicatorColor(void) {
          return ILI9341_BLUE;
  }
};

extern DefaultsClass Defaults; // Static instance to call constructor for initialization.

#endif // DEFAULTS_H_
