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

#include "HellaIbs.h"


//##############################################################################
// LIN frames
uint8_t linDiagRequestIbsGen1_123[] = {0x01, 0x06, 0xB2, 0x00, 0x36, 0x00, 0x0a, 0xf1};
uint8_t linDiagRequestIbsGen2_12_[] = {0x01, 0x06, 0xB2, 0x00, 0x36, 0x00, 0x0a, 0xf6};
uint8_t linDiagRequestIbsGen2___3[] = {0x01, 0x06, 0xB2, 0x00, 0x37, 0x00, 0x0a, 0xf6};


const uint8_t IBS_MAX_COUNT = 4;
uint8_t ibsTypeIndex = 0;
const IbsType ibs[IBS_MAX_COUNT] = {
        {0x01, linDiagRequestIbsGen2___3, true, "Gen2 Var. 3      ", 0x22, 7, 0x25, 6, 0x26, 6},
        {0x01, linDiagRequestIbsGen2_12_, true, "Gen2 Var. 1-2    ", 0x22, 7, 0x25, 6, 0x26, 6},
        {0x02, linDiagRequestIbsGen1_123, false, "Gen2 Var. 1-3, S2", 0x28, 7, 0x2B, 6, 0x2C, 6},
        {0x01, linDiagRequestIbsGen1_123, false, "Gen1 Var. 1-3, S1", 0x22, 7, 0x25, 6, 0x26, 6},
};


// Hella IBS diagnostic configuration frames
uint8_t linDiagIbsRetrieveNominalCapacity[] = {0x01 /* change NAD according to IBS type */, 0x02, 0xB2, 0x39, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t linDiagIbsWriteNominalCapacity[] = {0x01 /* change NAD according to IBS type */, 0x03, 0xB5, 0x39, 0x00 /* set before sending! */, 0xFF, 0xFF, 0xFF};

uint8_t linDiagIbsRetrieveBatteryType[] = {0x01 /* change NAD according to IBS type */, 0x06, 0xB2, 0x3A, 0xFF, 0x7F, 0xFF, 0xFF};
uint8_t linDiagIbsWriteBatteryType[] = {0x01 /* change NAD according to IBS type */, 0x03, 0xB5, 0x3A, 0x00 /* set before sending! */, 0xFF, 0xFF, 0xFF};

uint8_t linDiagIbsRetrieveBattTableState[] = {0x01 /* change NAD according to IBS type */, 0x01, 0x30, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const uint8_t MESSAGE_DELAY = 60; // msec


void HellaIbs::setup(LinDriver* linDriver) {
        this->linDriver = linDriver;
}

void HellaIbs::changeLoopState(HellaIbsLoopState newState) {
        loopState = newState;
        lastLoopStateChange = millis();
}

void HellaIbs::loop(void) {
        uint16_t durationSinceLastStateChange =  millis() - lastLoopStateChange;

        switch (loopState) {

        //######################################
        case kHilsNoIbsConnected:
                ibsTypeIndex = 0;
                connectedIbsIndex = 0xff;
                available = false;

                if (1000 < durationSinceLastStateChange) {
                        // Checking for IBS, testing its initial NAD, supplier ID and function ID
                        if (linDriver) {
                                changeLoopState(kHilsRequestIbs);
                        }
                }
                break;

        case kHilsRequestIbs:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        // Checking for IBS, testing its supplier ID, function ID and variant
                        ibs[ibsTypeIndex].diagRequestFrame[0] = ibs[ibsTypeIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, ibs[ibsTypeIndex].diagRequestFrame, 8);
                        changeLoopState(kHilsResponseIbs);
                }
                break;

        case kHilsResponseIbs:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        clearLinInfoFrame();
                        result = linDriver->recv(0x3D, linInfoFrame, 8);
                        if (0 == result) { // No answer?
                                ++ibsTypeIndex;
                                if (ibsTypeIndex >= IBS_MAX_COUNT) {
                                        changeLoopState(kHilsNoIbsConnected);
                                } else {
                                        changeLoopState(kHilsRequestIbs);
                                }
                        } else {
                                connectedIbsIndex = ibsTypeIndex;
                                available = true;
                                variant = linInfoFrame[7];
                                changeLoopState(kHilsRequestBatteryType);
                                Serial.printf("IBS found: %s\r\n", getName().c_str());
                        }
                }
                break;

        case kHilsRequestBatteryType:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        linDiagIbsRetrieveBatteryType[0] = ibs[ibsTypeIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, linDiagIbsRetrieveBatteryType, sizeof linDiagIbsRetrieveBatteryType);
                        changeLoopState(kHilsResponseBatteryType);
                }
                break;

        case kHilsResponseBatteryType:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        clearLinInfoFrame();
                        result = linDriver->recv(0x3D, linInfoFrame, 8);
                        if (0 == result) { // No answer?
                                changeLoopState(kHilsNoIbsConnected);
                        } else {
                                batteryType = (IbsBatteryType) linInfoFrame[3];
                                changeLoopState(kHilsPrepareReadStats);
                        }
                }
                break;


        //######################################
        case kHilsPrepareReadStats:
                changeLoopState(kHilsReadFrame2);
                break;

        case kHilsReadFrame2:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        clearLinInfoFrame();
                        result = linDriver->recv(ibs[connectedIbsIndex].frm2, linInfoFrame, ibs[connectedIbsIndex].frm2length);
                        if (0 == result) { // No answer?
                                changeLoopState(kHilsNoIbsConnected);
                        } else {
                                batteryVoltage = (float((linInfoFrame[4] << 8) + linInfoFrame[3])) / 1000; // V
                                batteryCurrent = (float((int32_t(linInfoFrame[2]) << 16) + (int32_t(linInfoFrame[1]) << 8) + int32_t(linInfoFrame[0]) - int32_t(2000000L))) / 1000.0; // A
                                temperature = long(linInfoFrame[5]) / 2 - 40;
                                ibsError = bitRead(linInfoFrame[6], 7);
                                changeLoopState(kHilsReadFrame5);
                        }
                }
                break;

        case kHilsReadFrame5:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        clearLinInfoFrame();
                        result = linDriver->recv(ibs[connectedIbsIndex].frm5, linInfoFrame, ibs[connectedIbsIndex].frm5length);
                        if (0 == result) { // No answer?
                                changeLoopState(kHilsNoIbsConnected);
                        } else {
                                soc = int(linInfoFrame[0]) / 2; // %
                                soh = int(linInfoFrame[1]) / 2; // %
                                optChargeVoltage = 13.0 + 0.05 * (linInfoFrame[3] - 160); // V
                                avgRi = (float) (linInfoFrame[4] | linInfoFrame[5] << 8) / 100; // mOhm
                                changeLoopState(kHilsReadFrame6);
                        }
                }
                break;

        case kHilsReadFrame6:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        clearLinInfoFrame();
                        result = linDriver->recv(ibs[connectedIbsIndex].frm6, linInfoFrame, ibs[connectedIbsIndex].frm6length);
                        if (0 == result) { // No answer?
                                changeLoopState(kHilsNoIbsConnected);
                        } else {
                                availableCapacity = (float) ((linInfoFrame[1] << 8) + linInfoFrame[0]) / 10; // Ah
                                dischargeableCapacity = (float) ((linInfoFrame[3] << 8) + linInfoFrame[2]) / 10; // Ah
                                nominalCapacity = linInfoFrame[4]; // Ah; TODO: *2 in case of newer IBS version...
                                calibrated = bitRead(linInfoFrame[5], 0);
                                changeLoopState(kHilsPauseAfterRead);
                        }
                }
                break;

        case kHilsPauseAfterRead:
                if (startConfigure) {
                        changeLoopState(kHilsPrepareConfig);
                } else if (2000 < durationSinceLastStateChange || (highSpeedCommunication && 200 < durationSinceLastStateChange)) { // Read battery stats once in two seconds.
                        changeLoopState(kHilsPrepareReadStats);
                }
                break;


        //######################################
        case kHilsPrepareConfig:
                startConfigure = false;
                changeLoopState(kHilsWriteConfigCapacityStep1);
                break;

        case kHilsWriteConfigCapacityStep1:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        // Please note: Sequence and minimum wait time between writes are very important.
                        // Start write capacity sequence:
                        linDiagIbsRetrieveNominalCapacity[0] = ibs[connectedIbsIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, linDiagIbsRetrieveNominalCapacity, 8);
                        changeLoopState(kHilsWriteConfigCapacityStep2);
                }
                break;

        case kHilsWriteConfigCapacityStep2:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        linDiagIbsWriteNominalCapacity[4] = configNominalCapacity;
                        linDiagIbsWriteNominalCapacity[0] = ibs[connectedIbsIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, linDiagIbsWriteNominalCapacity, 8);
                        changeLoopState(kHilsWriteConfigBattTypeStep1);
                }
                break;

        case kHilsWriteConfigBattTypeStep1:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        // Start write battery type sequence:
                        linDiagIbsRetrieveBatteryType[0] = ibs[connectedIbsIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, linDiagIbsRetrieveBatteryType, 8);
                        changeLoopState(kHilsWriteConfigBattTypeStep2);
                }
                break;

        case kHilsWriteConfigBattTypeStep2:
                if (MESSAGE_DELAY < durationSinceLastStateChange) {
                        // Start write battery type sequence:
                        linDiagIbsWriteBatteryType[4] = configBattType;
                        linDiagIbsWriteBatteryType[0] = ibs[connectedIbsIndex].nad; // Set correct NAD.
                        linDriver->send(0x3C, linDiagIbsWriteBatteryType, 8);
                        changeLoopState(kHilsRequestBatteryType);
                }
                break;


        //######################################
        default:
                changeLoopState(kHilsNoIbsConnected); // An unknown/unexpected error occurred, start over with IBS detection again.
                break;
        }
}

void HellaIbs::clearLinInfoFrame(void) {
        memset(linInfoFrame, 0x00, sizeof linInfoFrame);
}

void HellaIbs::configure(int16_t nominalCapacity /* Ah */, IbsBatteryType battType /* Std | Gel | AGM */) {
        startConfigure = true;
        configNominalCapacity = nominalCapacity;
        configBattType = battType;
}


String HellaIbs::getName(void) {
        if (available) {
                return ibs[ibsTypeIndex].name;
        } else {
                return String("unknown");
        }
}
