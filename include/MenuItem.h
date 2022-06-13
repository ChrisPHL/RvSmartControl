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

#ifndef MENU_ITEM_H_
#define MENU_ITEM_H_

#include "debug.h"
#include "defaults.h"
#include <Arduino.h>

#include <Adafruit_ILI9341.h>


#define RUN_LENGTH_DECODE(image_buf, rle_data, size, bpp) do \
        { uint8_t __bpp; uint8_t *__ip; const uint8_t *__il, *__rd; \
          __bpp = (bpp); __ip = (image_buf); __il = __ip + (size) * __bpp; \
          __rd = (rle_data); if (__bpp > 3) { /* RGBA */ \
                  while (__ip < __il) { uint8_t __l = *(__rd++); \
                                        if (__l & 128) { __l = __l - 128; \
                                                         do { memcpy (__ip, __rd, 4); __ip += 4; } while (--__l); __rd += 4; \
                                        } else { __l *= 4; memcpy (__ip, __rd, __l); \
                                                 __ip += __l; __rd += __l; } } \
          } else if (__bpp == 3) { /* RGB */ \
                  while (__ip < __il) { uint8_t __l = *(__rd++); \
                                        if (__l & 128) { __l = __l - 128; \
                                                         do { memcpy (__ip, __rd, 3); __ip += 3; } while (--__l); __rd += 3; \
                                        } else { __l *= 3; memcpy (__ip, __rd, __l); \
                                                 __ip += __l; __rd += __l; } } \
          } else { /* RGB16 */ \
                  while (__ip < __il) { uint8_t __l = *(__rd++); \
                                        if (__l & 128) { __l = __l - 128; \
                                                         do { memcpy (__ip, __rd, 2); __ip += 2; } while (--__l); __rd += 2; \
                                        } else { __l *= 2; memcpy (__ip, __rd, __l); \
                                                 __ip += __l; __rd += __l; } } \
          } } while (0)


class MenuItem {

private:
MenuItem* prevMenuItem;
MenuItem* nextMenuItem;

protected:
std::vector<bool> selectionFocus;
uint8_t selectionIndex;
boolean itemSelected;

Adafruit_ILI9341* adaIli9431;

String headline;
boolean menuEntered = false;

// Helper variables for font size measurements and formatted text display.
int16_t x1, y1;
uint16_t w, h;
char string[64];

/**
   This type definition mirrors the anonymous struct type which is created using GIMP's export as c-file function.
 */
typedef struct {
        uint32_t width;
        uint32_t height;
        uint32_t bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
        uint8_t rle_pixel_data[];
} RleImage;

public:
MenuItem(Adafruit_ILI9341* adaIli9431) {
        this->adaIli9431 = adaIli9431;
        this->headline = "unknown";
}

MenuItem(Adafruit_ILI9341* adaIli9431, String headline) {
        this->adaIli9431 = adaIli9431;
        this->headline = headline;
}

inline MenuItem* getPrevMenuItem(void) {
        return prevMenuItem;
}
void setPrevMenuItem(MenuItem* item) {
        prevMenuItem = item;
}
inline MenuItem* getNextMenuItem(void) {
        return nextMenuItem;
}
void setNextMenuItem(MenuItem* item) {
        nextMenuItem = item;
}

void commonPrintScreen(void) {
        menuEntered = false;
        adaIli9431->fillRect(0, 0, DISPLAY_W, SCROLLBAR_Y, Defaults.getBgColor());
        adaIli9431->setTextColor(Defaults.getFgColor(), Defaults.getBgColor());
        adaIli9431->setCursor(10, Defaults.getFontH());
        adaIli9431->print(headline);
}

virtual bool isVisible(void) = 0;
/**
   Implement this method and do whatever is necessary to build up the menu's screen (All static data/graphics.).
 */
virtual void printScreen(void) = 0;

/**
   Implement this method and do whatever is necessary to update the menu's screen (All dynamic data/graphics.).
 */
virtual void updateScreen(void) = 0;

/**
   Every menu can bentered by pushing the input button of the rotary controller. This method will be called if the menu is entered.
 */
virtual void onEnterMenu(void) = 0;

/**
   Every menu can bentered by pushing the input button of the rotary controller. When leaving the menu this
   method will be called and should implement saving changed data etc.
 */
virtual void onLeaveMenu(void) = 0;

virtual void inputLeft(void) = 0;
virtual void inputRight(void) = 0;
virtual void inputPush(void) = 0;

inline String getHeadline(void) {
        return headline;
}

inline boolean isMenuEntered(void) {
        return menuEntered;
}


void updateDisplayText(const char* s, uint16_t x, uint16_t y, uint16_t fgColor, uint16_t bgColor) {
        int16_t _x1, _y1;
        uint16_t _w, _h;
        adaIli9431->getTextBounds(s, 0, 0, &_x1, &_y1, &_w, &_h);
        uint16_t width = _x1 + _w + 1;
        // Serial.printf("%s, _x1: %d, _y1: %d, _w: %d, _h: %d\r\n", s, _x1, _y1, _w, _h);
        GFXcanvas1 canvas(width, Defaults.getFontH());
        canvas.setFont(Defaults.getFont());
        canvas.setCursor(0, Defaults.getFontY()+1);
        canvas.print(s);
        adaIli9431->drawBitmap(x, y - Defaults.getFontY(), canvas.getBuffer(), width, Defaults.getFontH(), fgColor, bgColor);
}

uint16_t getColorGradient(uint16_t color1, uint16_t color2, uint8_t percent) {
        uint16_t color1_red = (color1 & 0xF800) >> 11;
        uint16_t color1_green = (color1 & 0x07E0) >> 5;
        uint16_t color1_blue = (color1 & 0x001F);
        uint16_t color2_red = (color2 & 0xF800) >> 11;
        uint16_t color2_green = (color2 & 0x07E0) >> 5;
        uint16_t color2_blue = (color2 & 0x001F);
        double r, g, b;
        r = color1_red + percent * (color2_red - color1_red) / 100;
        g = color1_green + percent * (color2_green - color1_green) / 100;
        b = color1_blue + percent * (color2_blue - color1_blue) / 100;

        return (((uint16_t)r & 0x1F) << 11) | (((uint16_t)g & 0x3F) << 5) | ((uint16_t)b & 0x1F);
}

void drawCompressedImage(uint16_t x, uint16_t y, const RleImage* image) {
        GFXcanvas16* canvas = new GFXcanvas16(image->width, image->height);
        RUN_LENGTH_DECODE((uint8_t*)canvas->getBuffer(), image->rle_pixel_data, image->width * image->height, image->bytes_per_pixel);
        adaIli9431->drawRGBBitmap(x, y, canvas->getBuffer(), image->width, image->height);
        delete canvas;
}

};

#endif // MENU_ITEM_H_
