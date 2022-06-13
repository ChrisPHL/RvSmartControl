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

#include "HelpMenu.h"
#include "defaults.h"

void HelpMenu::printScreenImplementation(void) {
        commonPrintScreen();

        adaIli9431->setCursor(0, 3 * Defaults.getFontH());
        adaIli9431->println("Rotation left/right:");
        adaIli9431->println("  Change menu, change input");
        adaIli9431->println("  focus, change value.");
        adaIli9431->println();
        adaIli9431->println("Short press:");
        adaIli9431->println("  Enter menu, toggle focus");
        adaIli9431->println("  item.");
        adaIli9431->println();
        adaIli9431->println("Long press (>1 sec):");
        adaIli9431->println("  Leave menu, save settings.");
}

void HelpMenu::updateScreenImplementation(void) {

}

void HelpMenu::onEnterButtonImplementation(void) {

}

void HelpMenu::onLeaveButtonImplementation(void) {

}

void HelpMenu::inputLeft(void) {

}

void HelpMenu::inputRight(void) {

}

void HelpMenu::inputPush(void) {

}
