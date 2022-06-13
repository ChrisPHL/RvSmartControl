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

#ifndef WIFI_CONTROLLER_H_
#define WIFI_CONTROLLER_H_

#include "debug.h"
#include <Arduino.h>

typedef enum {
        kWclsIdle,
        kWclsStartup,
        kWclsDeInit,
        kWclsConfigInit,
        kWclsSetupSta,
        kWclsConnecting,
        kWclsSetupSmartConfig,
        kWclsWifiUpAndRunning
} WiFiControllerLoopState;


class WiFiController
{
  private:
    WiFiControllerLoopState loopState;
    uint64_t lastLoopStateChange;
    bool forceWiFiUpdate;
    bool wifiConfigEnable;


    WiFiController(void) {
            forceWiFiUpdate = false;
            wifiConfigEnable = false;
    }     // verhindert, dass ein Objekt von außerhalb von WifiController erzeugt wird.
    // protected, wenn man von der Klasse noch erben möchte
    WiFiController( const WiFiController& );     // verhindert, dass eine weitere Instanz via Kopier-Konstruktor erstellt werden kann
    WiFiController & operator = (const WiFiController &);     // Verhindert weitere Instanz durch Kopie

    void changeLoopState(WiFiControllerLoopState newState);
    void setupOtaUpdate(void);

  public:
    static WiFiController& getInstance() {
            static WiFiController instance;
            return instance;
    }

    void setup(void);
    /**
    @return true if WiFi update is forced, false otherwise.
    */
    bool loop(void);

    void start(void);
    void setWifiConfigEnable(void);
    void abortWifiConfig(void);
    WiFiControllerLoopState getState(void);
    void powerSave(void);
};


#endif // WIFI_CONTROLLER_H_
