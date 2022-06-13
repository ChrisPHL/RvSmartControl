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

#define WEB_SERVER_ENABLED true

RemoteDebug Debug;

#ifdef WEB_SERVER_ENABLED
#include <WebServer.h>
WebServer HTTPServer(80);

/////////// Handles
void handleRoot(void) {
        // Root web page
        HTTPServer.send(200, "text/plain", "hello from esp - RemoteDebug Sample!");
}

void handleNotFound(void){
        // Page not Found
        String message = "File Not Found\n\n";
        message.concat("URI: ");
        message.concat(HTTPServer.uri());
        message.concat("\nMethod: ");
        message.concat((HTTPServer.method() == HTTP_GET) ? "GET" : "POST");
        message.concat("\nArguments: ");
        message.concat(HTTPServer.args());
        message.concat("\n");
        for (uint8_t i=0; i<HTTPServer.args(); i++) {
                message.concat(" " + HTTPServer.argName(i) + ": " + HTTPServer.arg(i) + "\n");
        }
        HTTPServer.send(404, "text/plain", message);
}
#endif

// HardwareSerial Serial{1}; // UART3 (Serial2)
//
// DebugClass Debug;
//
// void DebugClass::setup(uint8_t debugTriggerPin) {
//         this->debugTrigger = debugTriggerPin;
//
//         if (0 <= debugTrigger) {
//                 pinMode(debugTrigger, OUTPUT);
//                 digitalWrite(debugTrigger, LOW);
//         } else {
//                 pinMode(debugTrigger, INPUT);
//         }
// }
//
// void ICACHE_RAM_ATTR DebugClass::trigger(uint8_t count) {
//         if (0 <= debugTrigger) {
//                 for (uint8_t q = 1; q <= count; ++q) {
//                         digitalWrite(debugTrigger, HIGH);
//                         digitalWrite(debugTrigger, LOW);
//                         if (0 == (q % 5)) {
//                                 delayMicroseconds(10);
//                         }
//                 }
//                 delayMicroseconds(25);
//         }
// }
