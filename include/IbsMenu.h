

#ifndef IBS_MENU_H_
#define IBS_MENU_H_

#include "debug.h"

#include "MenuItem.h"
#include "HellaIbs.h"
#include "MenuBatteryStatus.h"



//######################################

class IbsMenu : public MenuItem, public MenuBatteryStatus {

private:

void printScreenImplementation(void);
void updateScreenImplementation(void);
void onEnterButtonImplementation(void);
void onLeaveButtonImplementation(void);

void updateSelectionFocus(void);

HellaIbs* ibs;

public:
IbsMenu(Adafruit_ILI9341* adaIli9431, HellaIbs* ibs);
IbsMenu(Adafruit_ILI9341* adaIli9431, String headline, HellaIbs* ibs);

inline bool isVisible(void) {
        return ibs->isAvailable();
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

void drawBatteryIndicator(uint16_t x, uint16_t y, uint8_t w, uint8_t h, uint8_t poleH, uint8_t poleW);
void updateBatteryIndicator(uint16_t batteryX, uint16_t batteryY, uint16_t batteryWidth, uint16_t batteryHeight);
void updateNominalCapacityStat(uint16_t x, uint16_t y, int16_t capacity, boolean highlighted, boolean selected);
void updateBatteryTypeStat(uint16_t x, uint16_t y, IbsBatteryType batteryType, boolean highlighted, boolean selected);
};


#endif // IBS_MENU_H_
