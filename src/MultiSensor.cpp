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

#include "MultiSensor.h"


void MultiSensor::setup(void) {
        /*
           If CSB is connected to V DDIO, the I2C interface is active. If CSB is pulled down, the
           SPI interface is activated. After CSB has been pulled down once (regardless of whether any clock cycle occurred), the I2C
           interface is disabled until the next power-on-reset.
         */
        pinMode(this->pinSao, OUTPUT);
        digitalWrite(this->pinSao, HIGH);   // BMP280/MPU9250 address selection: HIGH: standard, LOW: alternate
        delay(2);
        pinMode(this->pinPwr, OUTPUT);
        digitalWrite(this->pinPwr, HIGH);   // enable: HIGH
        delay(2);

        Wire.begin(this->pinI2cSda, this->pinI2cScl);

        if (bmp280.begin(this->i2cAddressBmp, BMP280_CHIPID)) {
                Serial.println("BMP280 found, adjusting data sampling.");
                bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                                   Adafruit_BMP280::SAMPLING_X4,   /* Temp. oversampling */
                                   Adafruit_BMP280::SAMPLING_X16,   /* Pressure oversampling */
                                   Adafruit_BMP280::FILTER_X16,   /* Filtering. */
                                   Adafruit_BMP280::STANDBY_MS_500);   /* Standby time. */
        } else {
                Serial.println("BMP280 initialization failed.");
        }

        MPU9250Setting mpu9250Setting;
        mpu9250Setting.accel_fs_sel = ACCEL_FS_SEL::A16G;
        mpu9250Setting.gyro_fs_sel = GYRO_FS_SEL::G250DPS;
        mpu9250Setting.mag_output_bits = MAG_OUTPUT_BITS::M16BITS;
        mpu9250Setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_125HZ;
        mpu9250Setting.gyro_fchoice = 0x03;
        mpu9250Setting.gyro_dlpf_cfg = GYRO_DLPF_CFG::DLPF_92HZ;
        mpu9250Setting.accel_fchoice = 0x01;
        mpu9250Setting.accel_dlpf_cfg = ACCEL_DLPF_CFG::DLPF_45HZ;

        // Please take note of description at https://github.com/hideakitai/MPU9250.
        if (mpu9250.setup(this->i2cAddressMpu, mpu9250Setting, Wire)) {
                Serial.print("MPU9250 setup done, ");
                if (mpu9250.isConnected()) {
                        Serial.println("connected properly.");
                        mpu9250.verbose(true);
                        print_mpu9250_calibration();
                        Serial.println("Starting MPU9250 calibration.");
                        mpu9250.calibrateAccelGyro();
                        print_mpu9250_calibration();
                        Serial.println("MPU9250 found, starting self test.");
                        if (mpu9250.selftest()) {
                                Serial.println("MPU9250 self test successful.");
                        } else {
                                Serial.println("MPU9250 self test failed.");
                        }
                        print_mpu9250_calibration();
                        mpu9250.verbose(false);
                } else {
                        Serial.println("but conection failed.");
                }
        } else {
                Serial.println("MPU9250 initialization failed.");
        }


        // if (0 == mpu9250)
        //         mpu9250 = new MPU9250_asukiaaa(this->i2cAddressMpu);
        // if (0 == mpu9250) {
        //         Serial.println("Cannot instantiate MPU9250_asukiaaa.");
        // } else {
        //         Serial.println("MPU9250 found.");
        //         mpu9250->setWire(&Wire);
        //         mpu9250->beginAccel(ACC_FULL_SCALE_16_G);
        // }
}

void MultiSensor::changeLoopState(MultiSensorLoopState newState) {
        loopState = newState;
        lastLoopStateChange = millis();
}

void MultiSensor::loop(void) {
        uint16_t durationSinceLastStateChange =  millis() - lastLoopStateChange;

        readMpu9250Values();
}

void MultiSensor::print_mpu9250_calibration() {
        Serial.println("< calibration parameters >");
        Serial.println("accel bias [g]: ");
        Serial.print(mpu9250.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
        Serial.print(", ");
        Serial.print(mpu9250.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
        Serial.print(", ");
        Serial.print(mpu9250.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
        Serial.println();
        Serial.println("gyro bias [deg/s]: ");
        Serial.print(mpu9250.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
        Serial.print(", ");
        Serial.print(mpu9250.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
        Serial.print(", ");
        Serial.print(mpu9250.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
        Serial.println();
        Serial.println("mag bias [mG]: ");
        Serial.print(mpu9250.getMagBiasX());
        Serial.print(", ");
        Serial.print(mpu9250.getMagBiasY());
        Serial.print(", ");
        Serial.print(mpu9250.getMagBiasZ());
        Serial.println();
        Serial.println("mag scale []: ");
        Serial.print(mpu9250.getMagScaleX());
        Serial.print(", ");
        Serial.print(mpu9250.getMagScaleY());
        Serial.print(", ");
        Serial.print(mpu9250.getMagScaleZ());
        Serial.println();
}

void MultiSensor::readMpu9250Values(void) {
        if (millis() - lastSensorRead < 500) {
                return;
        }
        lastSensorRead = millis();
        // print_roll_pitch_yaw
        if (mpu9250.update()) {
                // Serial.print("Yaw, Pitch, Roll, Temp: ");
                // Serial.print(mpu9250.getYaw(), 2);
                // Serial.print(", ");
                // Serial.print(mpu9250.getPitch(), 2);
                // Serial.print(", ");
                // Serial.print(mpu9250.getRoll(), 2);
                // Serial.println(mpu9250.getTemperature(), 1);
                // Serial.println(); // Add an empty line
                char sensorData[64];
                // snprintf(sensorData, sizeof sensorData, "lin_acc: x=%# 2.3f, y=%# 2.3f, z=%# 2.3f          ", mpu9250.getEulerX(), mpu9250.getEulerY(), mpu9250.getEulerZ());
                snprintf(sensorData, sizeof sensorData, "accel.: x=%# 2.3f, y=%# 2.3f, z=%# 2.3f          ", mpu9250.getAccX(), mpu9250.getAccY(), mpu9250.getAccZ());
                Serial.print(sensorData);
                Serial.print("\r");
        }

        // Serial.print("\tTemp(*C): ");
        // Serial.print(bmp280.readTemperature());
        //
        // Serial.print("\tPressure(hPa|mBar): ");
        // Serial.print(bmp280.readPressure()/100.0F);
        //
        // Serial.print("\tAltitude(m): ");
        // Serial.print(bmp280.readAltitude(1013.25)); // this should be adjusted to your local forecast.
        //
        // Serial.println(); // Add an empty line
        // Serial.println(); // Add an empty line

        // if (mpu9250->accelUpdate() == 0) {
        //         aX = mpu9250->accelX();
        //         aY = mpu9250->accelY();
        //         aZ = mpu9250->accelZ();
        //         // aSqrt = mpu9250->accelSqrt();
        //         Serial.print(" accelX: " + String(aX));
        //         Serial.print(" accelY: " + String(aY));
        //         Serial.print(" accelZ: " + String(aZ));
        //         // Serial.print(" accelSqrt: " + String(aSqrt));
        // } else {
        //         Serial.println("Cannod read accel values");
        // }
        // Serial.print("\r");
}

void MultiSensor::scanForMpu9250(void) {
        Serial.println("Searching for MPU9250 device...");

        uint8_t addrs[7] = {0};
        uint8_t device_count = 0;
        for (uint8_t i = 0x68; i < 0x70; ++i) {
                Wire.beginTransmission(i);
                if (Wire.endTransmission() == 0) {
                        addrs[device_count++] = i;
                        delay(10);
                }
        }
        Serial.print("Found ");
        Serial.print(device_count, DEC);
        Serial.println(" I2C devices");


        Serial.print("I2C addresses are: ");
        for (uint8_t i = 0; i < device_count; ++i) {
                Serial.print("0x");
                Serial.print(addrs[i], HEX);
                Serial.print(" ");
        }
        Serial.println();
}
