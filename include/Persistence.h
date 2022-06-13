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

#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_

#include "debug.h"
#include <Arduino.h>
#include <EEPROM.h>

const uint16_t USE_EEPROM_SIZE = 256; // max.: 4096
const uint8_t CHECKSUM_CALC_START_BYTE = 0xA5;

typedef enum {
        kPlsCheckup,
        kPlsEraseEeprom,
        kPlsIdle,
        kPlsCommitting,
} PersistenceLoopState;

typedef enum {
        kPSlotWiFiOnOff,
        kPSlotWiFiSsid,
        kPSlotWiFiPassword,
        kPSlotBtOnOff,
        kPSlotBluetoothPair1,
        kPSlotBluetoothPair2,
        kPSlotBluetoothPair3,
        kPSlotBluetoothPair4,
} PersistenceSlot;

typedef struct {
  PersistenceSlot id;
  uint16_t addr;
  uint8_t length;
} MemorySlot;


class Persistence
{
  private:
    PersistenceLoopState loopState;

    Persistence(void) {
    }     // verhindert, dass ein Objekt von außerhalb von Persistence erzeugt wird.
    // protected, wenn man von der Klasse noch erben möchte
    Persistence( const Persistence& );     // verhindert, dass eine weitere Instanz via Kopier-Konstruktor erstellt werden kann
    Persistence & operator = (const Persistence &);     // Verhindert weitere Instanz durch Kopie

    void printEeprom(void);
    void changeLoopState(PersistenceLoopState newState);
    bool eepromCheckup(void);
    uint8_t calculateEepromChecksum(PersistenceSlot slot);
    bool commit(PersistenceSlot slot);
    void calculateSlotConstraints(void); // Calculate start addresses according to MemorySlot definition.

  public:
    static Persistence& getInstance() {
            static Persistence instance;
            return instance;
    }

    void setup(void);
    void loop(void);
    void eraseEeprom(void);
    uint8_t getSlotLength(PersistenceSlot slot);
    uint8_t writeSlotBoolean(PersistenceSlot slot, const bool active);
    uint8_t writeSlot(PersistenceSlot slot, const String* string);
    uint8_t writeSlot(PersistenceSlot slot, char* data, uint8_t length);
    bool readSlotBoolean(PersistenceSlot slot);
    String readSlot(PersistenceSlot slot);
    uint8_t readSlot(PersistenceSlot slot, char* data, uint8_t length);
};


#endif // PERSISTENCE_H_
