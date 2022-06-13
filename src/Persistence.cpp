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

#include "Persistence.h"


static const uint8_t memorySlotsCount = 8;
MemorySlot slots[] {
        // start addresses are calculated dynamically in formatEeprom().
        // Every last byte of a slot is used for a crc8 checksum to verify the data.
        // So minimum lenght is 2 bytes!
        {kPSlotWiFiOnOff, 0x0000, 2},
        {kPSlotWiFiSsid, 0x0000, 33},
        {kPSlotWiFiPassword, 0x0000, 65},
        {kPSlotBtOnOff, 0x0000, 2},
        {kPSlotBluetoothPair1, 0x0000, 12},
        {kPSlotBluetoothPair2, 0x0000, 12},
        {kPSlotBluetoothPair3, 0x0000, 12},
        {kPSlotBluetoothPair4, 0x0000, 12},
};


void Persistence::setup(void) {
        Serial.println("Persistence::setup()");
        EEPROM.begin(USE_EEPROM_SIZE);
        // printEeprom();
        calculateSlotConstraints();
        changeLoopState(kPlsCheckup);
}

void Persistence::changeLoopState(PersistenceLoopState newState) {
        // Serial.print("PersistenceLoopState = ");
        Serial.println(newState);
        loopState = newState;
}

void Persistence::loop(void) {

        switch (loopState) {
        case kPlsCheckup:
                if (!eepromCheckup()) {
                        changeLoopState(kPlsEraseEeprom);
                } else {
                        changeLoopState(kPlsIdle);
                }
                break;

        case kPlsEraseEeprom:
                eraseEeprom();
                changeLoopState(kPlsCheckup);
                break;

        case kPlsIdle:
                break;

        case kPlsCommitting:
                break;

        default:
                break;
        }
}

uint8_t Persistence::getSlotLength(PersistenceSlot slot) {
        return slots[slot].length;
}


uint8_t Persistence::writeSlotBoolean(PersistenceSlot slot, const bool active) {
        EEPROM.write(slots[slot].addr, (active ? 0x01 : 0xfe));
        if (!commit(slot)) {
                return 0;
        }
        return 1;
}

uint8_t Persistence::writeSlot(PersistenceSlot slot, char* data, uint8_t length = 0) {
        uint8_t ret_val = 0;
        if (0 == length || length > slots[slot].length) {
                length = slots[slot].length;
        }
        while(length--) {
                EEPROM.write(slots[slot].addr + ret_val, *data++);
                ++ret_val;
        }
        if (!commit(slot)) {
                ret_val = 0;
        }

        return ret_val;
}

uint8_t Persistence::writeSlot(PersistenceSlot slot, const String* string) {
        char* cstr = new char[string->length() + 1];
        strcpy(cstr, string->c_str());
        cstr[string->length()] = '\0';
        uint8_t ret_val = writeSlot(slot, &cstr[0], string->length() + 1);
        delete cstr;
        return ret_val;
}

bool Persistence::readSlotBoolean(PersistenceSlot slot) {
        uint8_t byte = EEPROM.read(slots[slot].addr);
        if (0x01 == byte) {
                return true;
        }
        return false;
}

String Persistence::readSlot(PersistenceSlot slot) {
        char data[slots[slot].length + 1];
        readSlot(slot, data, slots[slot].length);
        data[slots[slot].length] = '\0';
        return data;
}

uint8_t Persistence::readSlot(PersistenceSlot slot, char* data, uint8_t length = 0) {
        uint8_t k = 0;
        uint8_t len = length;
        if (0 == length) {
                len = slots[slot].length;
        }
        for (k=0; k<len; ++k) {
                data[k] = EEPROM.read(slots[slot].addr + k);
        }
        return k;
}

void Persistence::calculateSlotConstraints(void) {
        Serial.println("calculateSlotConstraints()");
        uint16_t addr = 0x000;
        uint8_t length = 0x00;

        for (uint8_t x=0; x<memorySlotsCount; ++x) {
                slots[x].addr = addr + length;
                // Serial.print("Slot #");
                // Serial.print(x);
                // Serial.printf(": Addr.: %#05x, length: %4d.\r\n", slots[x].addr, slots[x].length);

                length += slots[x].length;
        }
        if (slots[memorySlotsCount-1].addr + slots[memorySlotsCount-1].length > USE_EEPROM_SIZE) {
                Serial.println("MemorySlots exceeding USE_EEPROM_SIZE.");
                while(true) {
                        asm ("nop");
                }
        }
}

void Persistence::printEeprom(void) {
        char buffer[6];
        uint8_t units_per_line = 16;

        for (uint16_t z=0; z<USE_EEPROM_SIZE; ++z) {
                snprintf(buffer, sizeof buffer, "0x%02x ", EEPROM.read(z));
                Serial.print(buffer);
                if (units_per_line - 1 == z % units_per_line) {
                        Serial.println();
                } else if (4-1 == (z % units_per_line) % 4) {
                        if (16-1 == (z % units_per_line) % 16) {
                                Serial.println();
                        } else {
                                Serial.print(" ");
                        }
                }
        }
        Serial.println();
}

bool Persistence::commit(PersistenceSlot slot) {
        Serial.print("commit() slot: ");
        Serial.println(slot);
        EEPROM.write(slots[slot].addr + slots[slot].length - 1, calculateEepromChecksum(slot));
        return EEPROM.commit();
}

bool Persistence::eepromCheckup(void) {
        Serial.println("eepromCheckup()");
        bool ret_val = true;
        for (uint8_t s=0; s<memorySlotsCount; ++s) {
                if (calculateEepromChecksum((PersistenceSlot)s) != EEPROM.read(slots[s].addr + slots[s].length - 1)) {
                        ret_val = false;
                        Serial.print("Checksum error on Persistence slot ");
                        Serial.print(s);
                        Serial.println(".");
                        break;
                }
        }
        // Serial.print("ret_val=");
        // Serial.println(ret_val);
        return ret_val;
}

void Persistence::eraseEeprom(void) {
        Serial.println("eraseEeprom()");
        for (uint16_t addr = 0; addr<USE_EEPROM_SIZE; ++addr) {
                EEPROM.write(addr, 0xFE);
        }
        EEPROM.commit();
        for (uint8_t x=0; x<memorySlotsCount; ++x) {
                commit((PersistenceSlot)x);
        }
}

uint8_t Persistence::calculateEepromChecksum(PersistenceSlot slot) {
        // Serial.println("calculateEepromChecksum()");
        uint8_t checksum = CHECKSUM_CALC_START_BYTE;
        for (uint16_t addr = slots[slot].addr; addr<slots[slot].addr + slots[slot].length - 1; ++addr) {
                uint8_t data = EEPROM.read(addr);
                for (uint8_t i = 8; i; i--) {
                        uint8_t sum = (checksum ^ data) & 0x01;
                        checksum >>= 1;
                        if (sum) {
                                checksum ^= 0x8C;
                        }
                        data >>= 1;
                }
        }
        // Serial.print("checksum=");
        // Serial.println(checksum);
        return checksum;
}
