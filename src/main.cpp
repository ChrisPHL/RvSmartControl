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
#include <Arduino.h>

void print_wakeup_reason(void);
void setupDevices(void);

bool sleeping = false;
bool remoteDebugSetupDone = false;

// RemoteDebug rDebug;

//##############################################################################
// Configuration
// Please note the pin reference at
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
// So pulled up or pulled low pins should be used for output only.
// e.g. Pins that are pulled up _cannot_ be used for pulling low inputs like rotary button!
//
// For ESP32 please note (https://www.electroniclinic.com/esp32-wroom-32d-pinout-features-and-specifications/):
// Input/output Pins - GPIO0 to GPIO39 - Totally 39 GPIO pins, can be used as input or output pins. 0V (low) and 3.3V (high). But pins 34 to 39 can be used as input only

//######################################
#include "WiFiController.h"


//######################################
#include "LinDriver.h"
const uint8_t LIN_TXE_PIN = 15;
const uint32_t LIN_BAUDRATE = 19200;
const uint8_t LIN_TX_PIN = 17;
const uint8_t LIN_RX_PIN = 16;
LinDriver linDriver{&Serial2, LIN_TXE_PIN, LIN_RX_PIN, LIN_TX_PIN, LIN_BAUDRATE};


//######################################
#include "GfxMenu.h"
// ST7735S pinout description:
// http://www.lcdwiki.com/1.8inch_Arduino_SPI_Module_ST7735S_SKU:MAR1801
const uint8_t TFT_PIN_CS = 32; // CS: LCD chip select signal, low level enable
const uint8_t TFT_PIN_DCX = 33; // RS: LCD register / data selection signal, high level: register, low level: data
const uint8_t TFT_PIN_RST = 25; // D4 for RELEASE mode, -1 for DEBUG RST: LCD reset signal, low level reset (-1 for n/a) (Note: D4 is used for TXD1 also.)
const uint8_t TFT_PIN_BL = 26;  // Backlight On/Off.
const uint8_t TFT_PIN_PWR = 27;  // Power supply for the TFT (direct driven). Choose zero for external powered TFT unit.
GfxMenu gfxMenu{TFT_PIN_CS, TFT_PIN_DCX, TFT_PIN_RST, TFT_PIN_BL, TFT_PIN_PWR}; // FIXME: Static use of GfxMenu here, new GfxMenu(...) does not seem to work here, display does not initialize. :-/

//######################################
#include "MultiSensor.h"
const uint8_t I2C_SDA_PIN = 21;
const uint8_t I2C_SCL_PIN = 22;
const uint8_t BMP280_MPU9250_PIN_PWR = 13;
const uint8_t BMP280_MPU9250_PIN_SAO = 15;
const uint8_t BMP280_I2C_ADDRESS = 0x77;
const uint8_t MPU9250_I2C_ADDRESS = 0x69;
MultiSensor multiSensor(I2C_SDA_PIN, I2C_SCL_PIN, BMP280_MPU9250_PIN_PWR, BMP280_MPU9250_PIN_SAO, BMP280_I2C_ADDRESS, MPU9250_I2C_ADDRESS);

//######################################
#include "PowerSaver.h"
void powerSaveReturnMenu(void);
void powerSaveSleep(void);
void powerSaveWakeUp(void);
PowerSaver powerSaver;

//######################################
#include "RotaryCJMCU_111.h"
const uint8_t ROTARY_PIN_GA = 35;
const uint8_t ROTARY_PIN_GB = 34;

//######################################
// LIN
const uint8_t LIN_CS = 12;
const uint8_t LIN_PWR = 14;

//######################################
#include "Persistence.h"

//##############################################################################
void setup(void)
{
        uint64_t startup_response_delay = millis();
        Serial.begin(DEBUG_SERIAL_BAUDRATE, SERIAL_8N1, DEBUG_SERIAL_RX_PIN, DEBUG_SERIAL_TX_PIN);
        Serial.setDebugOutput(true);
        Serial.println();
        Serial.println("+------------------------------------------------------------------------------+");
        Serial.println("|                              RV Smart Control                                |");
        Serial.println("+------------------------------------------------------------------------------+");
        Serial.printf("Startup response delay: %lld msec.\r\n", startup_response_delay);
        print_wakeup_reason();

        sleeping = false;


        RotaryCJMCU_111::getInstance().setup(ROTARY_PIN_GA, ROTARY_PIN_GB);
        if (RotaryCJMCU_111::getInstance().isButtonPushed()) {
                // TODO: Let the user decide by long press if we shall use default WiFi data or try to read WiFi credentials from EEPROM.
                Serial.println("Button pushed initially. Just starting WiFi and Rotary button controller.");
                RotaryCJMCU_111::getInstance().setup(ROTARY_PIN_GA, ROTARY_PIN_GB);
                WiFiController::getInstance().setup();
                WiFiController::getInstance().start();
                return;
        }

        setupDevices();

        Serial.println("Setup done.");
}

void setupDevices(void) {
        Persistence::getInstance().setup();
        WiFiController::getInstance().setup();
        WiFiController::getInstance().start();

        gfxMenu.setup(&linDriver);

        powerSaver.setup(300, 30, powerSaveReturnMenu, powerSaveSleep);

        multiSensor.setup();

        pinMode(LIN_PWR, OUTPUT);
        digitalWrite(LIN_PWR, HIGH); // enable: HIGH
        pinMode(LIN_CS, OUTPUT);
        digitalWrite(LIN_CS, HIGH); // enable: HIGH
}

//##############################################################################
void loop(void)
{
        if (sleeping) {
                yield();
                return;
        }


        //######################################
        if (RotaryCJMCU_111::getInstance().isRotaryInputDetected()) {
                Serial.println("Input");
                powerSaver.resetTimer();

                uint32_t l, r, p, lp;
                noInterrupts();
                l = RotaryCJMCU_111::getInstance().getRotationCountL();
                r = RotaryCJMCU_111::getInstance().getRotationCountR();
                p = RotaryCJMCU_111::getInstance().getRotaryPushCount();
                lp = RotaryCJMCU_111::getInstance().getRotaryLongPressCount();
                RotaryCJMCU_111::getInstance().resetRotaryInput();
                interrupts();

                if (0 < l) {
                        Serial.print("L ");
                        Serial.println(l);
                        gfxMenu.inputLeft(l);
                } else if (0 < r) {
                        Serial.print("R ");
                        Serial.println(r);
                        gfxMenu.inputRight(r);
                } else if (0 < p) {
                        Serial.print("P ");
                        Serial.println(p);
                        gfxMenu.inputPush(p);
                } else if (0 < lp) {
                        Serial.print("LP ");
                        Serial.println(lp);
                        gfxMenu.inputLongPress(lp);

                        // calibrate anytime you want to
                        // Serial.println("Accel Gyro calibration will start in 5sec.");
                        // Serial.println("Please leave the device still on the flat plane.");
                        // mpu9250.verbose(true);
                        // delay(5000);
                        // mpu9250.calibrateAccelGyro();

                        // Serial.println("Mag calibration will start in 5sec.");
                        // Serial.println("Please Wave device in a figure eight until done.");
                        // delay(5000);
                        // mpu9250.calibrateMag();

                        // print_mpu9250_calibration();
                        // mpu9250.verbose(false);
                }
        }

        RotaryCJMCU_111::getInstance().loop();


        if (WiFiController::getInstance().loop()) {
                if (300000 < millis()) {
                        Serial.println("300 sec elapsed, rebooting.");
                        ESP.restart();
                }
                return;
        }


        if (!remoteDebugSetupDone && kWclsWifiUpAndRunning == WiFiController::getInstance().getState()) {
                // WiFi connection is essential for starting the Remote Debugging Interface.
                Serial.println("Setting up Remote Debugging Interface.");
                Debug.begin("rvsmartcontrol"); // OR: Debug.begin(HOST_NAME, startingDebugLevel);
                Debug.setResetCmdEnabled(true); // Enable the reset command
                Debug.showProfiler(true);   // Profiler (Good to measure times, to optimize codes)
                Debug.showColors(true);   // Colors

#ifdef WEB_SERVER_ENABLED
                HTTPServer.on("/", handleRoot);
                HTTPServer.onNotFound(handleNotFound);
                HTTPServer.begin();
#endif
                remoteDebugSetupDone = true;
        }
        if (remoteDebugSetupDone) {
                Debug.handle(); // Remote debug over WiFi
                // Debug.debugHandle(); // Equal to SerialDebug
                // debugA("This is a any (always showed) - var %lu", millis());

#ifdef WEB_SERVER_ENABLED
                // Web server
                HTTPServer.handleClient();
#endif
        }

        Persistence::getInstance().loop();

        gfxMenu.loop();

        powerSaver.loop();

        multiSensor.loop();

        yield();
}


//##############################################################################
// Power save functions

/**
   Function that prints the reason by which ESP32 has been awaken from sleep
   This i inspired by https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/.
 */
void print_wakeup_reason(void){
        esp_sleep_wakeup_cause_t wakeup_reason;
        wakeup_reason = esp_sleep_get_wakeup_cause();
        switch(wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
        case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
        case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
        case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
        case ESP_SLEEP_WAKEUP_GPIO: Serial.println("Wakeup caused by GPIO (light sleep only)"); break;
        case ESP_SLEEP_WAKEUP_UART: Serial.println("Wakeup caused by UART (light sleep only)"); break;
        default: Serial.println("Wakeup was _not_ caused by deep sleep"); break;
        }
}

void powerSaveReturnMenu(void) {
        Serial.println("powerSaveReturnMenu()");
        gfxMenu.timeout();
}

/**
   This i inspired by https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/.
 */
void powerSaveSleep(void) {
        Serial.println("powerSaveSleep()");
        sleeping = true;
        gfxMenu.tftPowerDown();

        WiFiController::getInstance().powerSave();

        digitalWrite(BMP280_MPU9250_PIN_PWR, LOW);

        digitalWrite(LIN_PWR, LOW);
        digitalWrite(LIN_CS, LOW);

        Serial.flush();

        esp_sleep_enable_ext1_wakeup(((uint64_t)1 << ROTARY_PIN_GA), ESP_EXT1_WAKEUP_ALL_LOW);

        //esp_light_sleep_start(); // Code execution stops at this line.
        esp_deep_sleep_start(); // Code execution stops at this line.
}
