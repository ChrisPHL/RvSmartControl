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

#ifndef TRUMA_COMBI_MENU_H_
#define TRUMA_COMBI_MENU_H_

#include "debug.h"
#include "MenuItem.h"


class TrumaCombiMenu : public MenuItem {

private:

void printScreenImplementation(void);
void updateScreenImplementation(void);
void onEnterButtonImplementation(void);
void onLeaveButtonImplementation(void);

public:

TrumaCombiMenu(Adafruit_ILI9341* adaIli9431)
        : MenuItem(adaIli9431) {
}

TrumaCombiMenu(Adafruit_ILI9341* adaIli9431, String headline)
        : MenuItem(adaIli9431, headline) {
}

inline bool isVisible(void) {
        return false; // TODO: Implement according to LIN device available.
}

void printScreen(void) {
        printScreenImplementation();
}
void updateScreen(void) {
        updateScreenImplementation();
}

void onEnterMenu(void) {
        onEnterButtonImplementation();
}

void onLeaveMenu(void) {
        onLeaveButtonImplementation();
}

void inputLeft(void);
void inputRight(void);
void inputPush(void);
};


#endif // TRUMA_COMBI_MENU_H_
