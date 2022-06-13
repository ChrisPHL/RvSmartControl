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

 #include "defaults.h"
 #include "debug.h"

DefaultsClass Defaults;

DefaultsClass::DefaultsClass(void) {
        font_h = 2;
        font_y = 1;
        font_char_w = 0;
};


void DefaultsClass::setup(Adafruit_ILI9341* gfxContext) {
        int16_t x1, y1;
        uint16_t w, h;
        // Calculate font heights using some character with maximum spreading:
        gfxContext->setFont(getFont());
        gfxContext->getTextBounds("Qy", 0, 0, &x1, &y1, &w, &h);
        font_h = h + 1;
        font_y = -y1;
        font_char_w = w / 2;

        Serial.printf("Defaults: font_h: %d, font_y: %d, font_char_w: %d\r\n", Defaults.getFontH(), Defaults.getFontY(), Defaults.getFontCharW());
}
