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

#include "WiFiController.h"
#include "Persistence.h"
#include "RotaryCJMCU_111.h"

#include <WiFi.h>
#include <ArduinoOTA.h>

static uint64_t smartConfigStartTime = 0;

void WiFiController::setup(void) {
        Serial.println("WiFiController::setup()");
        WiFi.disconnect(/* wifioff */ true, /* eraseap */ true);
        changeLoopState(kWclsIdle);
}

void WiFiController::changeLoopState(WiFiControllerLoopState newState) {
        Serial.print("WiFiControllerLoopState = ");
        Serial.println(newState);
        loopState = newState;
        lastLoopStateChange = millis();
}

bool WiFiController::loop(void) {
        uint64_t currentTime = millis();
        switch (loopState) {
        case kWclsIdle:
                // Nothing to do...
                break;

        case kWclsStartup:
                if (RotaryCJMCU_111::getInstance().isButtonPushed() && !wifiConfigEnable) {
                        forceWiFiUpdate = true;
                        if ((currentTime - lastLoopStateChange) > 5000) {
                                changeLoopState(kWclsDeInit);
                        }
                } else if ((currentTime - lastLoopStateChange) > 2500) { // On POR we want to avoid current peaks, so activating WiFi hardware some seconds later...
                        // static const String wifi_ssid = "gateway.chpohl.home";
                        // static const String wifi_pass = "lAn4cc&ss?privateOnly";
                        // Persistence::getInstance().writeSlot(kPSlotWiFiSsid, &wifi_ssid);
                        // Persistence::getInstance().writeSlot(kPSlotWiFiPassword, &wifi_pass);
                        forceWiFiUpdate = false;
                        changeLoopState(kWclsDeInit);
                }
                break;

        case kWclsDeInit:
                WiFi.mode(WIFI_STA);
                WiFi.disconnect(true, true);
                changeLoopState(kWclsConfigInit);
                break;

        case kWclsConfigInit:
                if (Persistence::getInstance().readSlotBoolean(kPSlotWiFiOnOff) || forceWiFiUpdate || wifiConfigEnable) {
                        changeLoopState(kWclsSetupSta);
                }
                break;

        case kWclsSetupSta:
                // WiFi.setTxPower(WIFI_POWER_19_5dBm); // Be sure to leave this state after one cycle.
                if ((forceWiFiUpdate) || wifiConfigEnable) {
                        // Emergency WiFi connect:
                        Serial.println("Starting Smart Config, please use EspTouchDemo App to configure\r\nRvSmartControl's WiFi:");
                        Serial.println("https://github.com/EspressifApp/EsptouchForAndroid/releases");
                        Serial.println("The device running the app may be connected to the 5 GHz WiFi as\r\nlong as the SSID and the network are the same.");
                        WiFi.beginSmartConfig();
                        smartConfigStartTime = millis();
                        changeLoopState(kWclsSetupSmartConfig);
                } else {
                        WiFi.begin(
                                Persistence::getInstance().readSlot(kPSlotWiFiSsid).c_str(),
                                Persistence::getInstance().readSlot(kPSlotWiFiPassword).c_str()
                                );
                        changeLoopState(kWclsConnecting);
                }
                break;

        case kWclsSetupSmartConfig:
                if (WiFi.smartConfigDone()) {
                        Serial.println("Smart Config done.");
                        WiFi.stopSmartConfig();
                        changeLoopState(kWclsConnecting);
                } else {
                        // Abort WiFi Smart Config after 90 sec.
                        if (millis() - smartConfigStartTime > 90000) {
                                wifiConfigEnable = false;
                                start();
                        }
                }
                break;

        case kWclsConnecting:
                if (WiFi.isConnected()) {
                        Serial.print("WiFi connected to ");
                        Serial.println(WiFi.SSID());
                        Serial.print("IP address: ");
                        Serial.print(WiFi.localIP());
                        Serial.print(", MAC address: ");
                        Serial.println(WiFi.macAddress());
                        if (wifiConfigEnable) {
                                // We assume WiFi config mode running only on normal operation so EEPROM is setup properly.
                                Serial.println("Storing WiFi setup to EEPROM.");
                                String ssid = WiFi.SSID();
                                String psk = WiFi.psk();
                                Persistence::getInstance().writeSlot(kPSlotWiFiSsid, &ssid);
                                Persistence::getInstance().writeSlot(kPSlotWiFiPassword, &psk);
                        }
                        setupOtaUpdate();
                        changeLoopState(kWclsWifiUpAndRunning);
                } else if ((currentTime - lastLoopStateChange) > 16000) {
                        Serial.println("WiFi does not connect in time, start retrying...");
                        changeLoopState(kWclsDeInit);
                }
                break;

        case kWclsWifiUpAndRunning:
                ArduinoOTA.handle();
                break;

        default:
                break;
        }

        return forceWiFiUpdate;
}

void WiFiController::setupOtaUpdate(void) {
        // Port defaults to 3232
        // ArduinoOTA.setPort(3232);

        // Hostname defaults to esp3232-[MAC]
        ArduinoOTA.setHostname("rvsmartcontrol");

        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // MD5 sum can be created like this: 'echo -n "passwd" | md5sum'.
        ArduinoOTA.setPasswordHash("8a5aca79af1a4456ecb40480666b2fd1"); // rscConne3ct

        ArduinoOTA
        .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                        type = "sketch";
                else // U_SPIFFS
                        type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
        })
        .onEnd([]() {
                Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

        ArduinoOTA.begin();
}


void WiFiController::start(void) {
        changeLoopState(kWclsStartup);
}

void WiFiController::setWifiConfigEnable(void) {
        Serial.println("setWifiConfigEnable()");
        wifiConfigEnable = true;
        start();
}

void WiFiController::abortWifiConfig(void) {
        Serial.println("abortWifiConfig()");
        wifiConfigEnable = false;
        WiFi.stopSmartConfig();
        start();
}

WiFiControllerLoopState WiFiController::getState(void) {
        return loopState;
}

void WiFiController::powerSave(void) {
        WiFi.disconnect(true); // bool wifioff = false, optional bool eraseap = false
}
