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


#ifndef HELLA_IBS_H_
#define HELLA_IBS_H_

#include "debug.h"
#include <Arduino.h>
#include "LinDriver.h"
#include <string.h>


typedef struct {
  uint8_t nad; // Node ADress
  uint8_t* diagRequestFrame;
  bool doubleCapacity;
  String name;
  uint8_t frm2;
  uint8_t frm2length;
  uint8_t frm5;
  uint8_t frm5length;
  uint8_t frm6;
  uint8_t frm6length;
} IbsType;


typedef enum {
  kBatteryTypeStd = 0x0A,
  kBatteryTypeGel = 0x14,
  kBatteryTypeAgm = 0x1E,
} IbsBatteryType;


typedef enum {
  kHilsNoIbsConnected,

  kHilsRequestIbs, // Send diagnostic request frame.
  kHilsResponseIbs, // Recieve diagnostic response frame.
  kHilsRequestBatteryType,
  kHilsResponseBatteryType,

  kHilsPrepareReadStats,
  kHilsReadFrame2,
  kHilsReadFrame5,
  kHilsReadFrame6,
  kHilsPauseAfterRead,

  kHilsPrepareConfig,
  kHilsWriteConfigCapacityStep1,
  kHilsWriteConfigCapacityStep2,
  kHilsWriteConfigBattTypeStep1,
  kHilsWriteConfigBattTypeStep2,

} HellaIbsLoopState;

/**
   This class is an abstraction of the Hella IBS's (Intelligent Battery Sensor).
   It is built on top of the LIN driver created by 'gandrewstone'. The source of the LIN driver can be found here:
   https://github.com/gandrewstone/LIN
*/
class HellaIbs {

  private:
    uint8_t connectedIbsIndex;
    uint8_t variant;

    bool startConfigure;
    int16_t configNominalCapacity;
    IbsBatteryType configBattType;

    LinDriver* linDriver;

    uint8_t soc; // state of charge
    uint8_t soh; // state of health
    float avgRi; // average internal resistance of battery
    float optChargeVoltage; // optimum charging voltage
    float batteryVoltage;
    float batteryCurrent;
    float temperature;
    float availableCapacity;
    float dischargeableCapacity;
    int16_t nominalCapacity;
    bool calibrated = false;
    bool ibsError = false;
    bool highSpeedCommunication = false;
    bool available = false;

    IbsBatteryType batteryType;

    uint8_t result;
    uint8_t linInfoFrame[8];

    void clearLinInfoFrame(void);

    uint64_t lastLoopStateChange;
    HellaIbsLoopState loopState;

    void changeLoopState(HellaIbsLoopState newState);

  public:

    HellaIbs() {
      startConfigure = false;
      changeLoopState(kHilsNoIbsConnected);
    }

    void setup(LinDriver* linDriver);

    /**
       Call this method as fast as possible got guarantee fluent operation.
    */
    void loop(void);

    void configure(int16_t nominalCapacity /* Ah */, IbsBatteryType battType);

    inline void setHighSpeedCommunication(boolean enable) {
      highSpeedCommunication = enable;
    }

    inline uint8_t getSoc(void) {
      return soc;
    }

    inline uint8_t getSoh(void) {
      return soh;
    }

    inline uint8_t getAvgRi(void) {
      return avgRi;
    }

    inline bool isCalibrated(void) {
      return calibrated;
    }

    inline bool isError(void) {
      return ibsError;
    }

    inline IbsBatteryType getBatteryType(void) {
      return batteryType;
    }

    inline float getBatteryCurrent(void) {
      return batteryCurrent;
    }

    inline float getBatteryVoltage(void) {
      return batteryVoltage;
    }

    inline float getOptChargeVoltage(void) {
      return optChargeVoltage;
    }

    inline float getAvailableCapacity(void) {
      return availableCapacity;
    }

    inline float getDischargeableCapacity(void) {
      return dischargeableCapacity;
    }

    inline int16_t getNominalCapacity(void) {
      return nominalCapacity;
    }

    inline float getTemperature(void) {
      return temperature;
    }

    inline bool isAvailable(void) {
      return available;
    }

    String getName(void);
};


#endif // HELLA_IBS_H_
