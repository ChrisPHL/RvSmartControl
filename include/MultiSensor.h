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


#ifndef MULTISENSOR_H_
#define MULTISENSOR_H_

#include "debug.h"
#include <Arduino.h>

#include <MPU9250.h>
// #include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>


typedef enum {
        kMslsNotConnected,
        kMslsIntializing,
        kMslsRunning,
} MultiSensorLoopState;

/**
   This class is an abstraction of the MPU9250 + BMP280 combi sensor chip.
 */
class MultiSensor {
private:
        MultiSensorLoopState loopState;
        long lastLoopStateChange;

        uint8_t pinI2cSda;
        uint8_t pinI2cScl;
        uint8_t pinPwr;
        uint8_t pinSao;
        uint8_t i2cAddressBmp;
        uint8_t i2cAddressMpu;

        Adafruit_BMP280 bmp280; // using I2C
        MPU9250 mpu9250;
        // MPU9250_asukiaaa* mpu9250;
        float aX, aY, aZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ;

        uint64_t lastSensorRead;

        void print_mpu9250_calibration(void);
        void readMpu9250Values(void);
        void scanForMpu9250(void);

public:
        MultiSensor(uint8_t pinI2cSda, uint8_t pinI2cScl, uint8_t pinPwr, uint8_t pinSao, uint8_t i2cAddressBmp, uint8_t i2cAddressMpu) {
                this->pinI2cSda = pinI2cSda;
                this->pinI2cScl = pinI2cScl;
                this->pinPwr = pinPwr;
                this->pinSao = pinSao;
                this->i2cAddressBmp = i2cAddressBmp;
                this->i2cAddressMpu = i2cAddressMpu;

                loopState = kMslsNotConnected;
                lastLoopStateChange = 0;
                lastSensorRead = millis();
        }

        void setup(void);
        void changeLoopState(MultiSensorLoopState newState);
        void loop(void);
};


#endif // MULTISENSOR_H_
