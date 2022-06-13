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

#ifndef DEBUG_H_
#define DEBUG_H_

#include "RemoteDebug.h"
#include <Arduino.h>



extern RemoteDebug Debug;

#ifndef WEBSOCKET_DISABLED // Only if Web socket enabled (RemoteDebugApp)
// If enabled, you can change the port here (8232 is default)
// Uncomment the line below, to do it:
//#define WEBSOCKET_PORT 8232

// Internally, the RemoteDebug uses a local copy of the arduinoWebSockets library (https://github.com/Links2004/arduinoWebSockets)
// Due it not in Arduino Library Manager
// If your project already use this library,
// Uncomment the line below, to do it:
//#define USE_LIB_WEBSOCKET true
#endif
#ifdef WEB_SERVER_ENABLED
#info "WEB_SERVER_ENABLED"
extern WebServer HTTPServer(80);

void handleRoot(void);
void handleNotFound(void);
#endif

const uint32_t DEBUG_SERIAL_BAUDRATE = 115200;
const uint8_t DEBUG_SERIAL_TX_PIN = 2;
const uint8_t DEBUG_SERIAL_RX_PIN = 4;

// class DebugClass {
// private:
//   int8_t debugTrigger;
// public:
//   DebugClass(void) {
//     debugTrigger = -1;
//   };
//
//   void setup(uint8_t debugTriggerPin);
//   void trigger(uint8_t count);
// };
//
//
// extern HardwareSerial Serial;
//
// extern DebugClass Debug; // Static instance to call constructor for initialization.

#endif // DEBUG_H_
