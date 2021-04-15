# RV Smart Control
RV Smart Control is a LIN-bus based control system for recreational vehicles. It runs on an ESP32 (NodeMCU-ESP32/ESP32 DEVKITV1), utilizes an extra *LINBUS Breakout board* and a *CJMCU-111* rotary input device. (I wrote an Arduino demo which shows the latter one interrupt driven.)
Furhtermore an *ILI9341 Display* (320x240 px) is used to show the menu driven control and a combined sensor board is connected to aquire environmental information.

## Part list
* NodeMCU-ESP32/ESP32 DEVKITV1
* LINBUS Breakout board http://skpang.co.uk/catalog/linbus-breakout-board-p-1417.html
* USB Step Down DC-DC Spannungswandler 6-24V zu 5V 3A Arduino Spannungsregler
* 2.8 Inch ILI9341 240x320 SPI TFT LCD Display Touch Panel SPI Serial Port Module
* CJMCU-111 DC 5V Push Button Switch Rotary Encoder / Momentary Push Switch
* GY-91 10DOF Accelerometer Gyroscope Compass Temp/Pressure MPU-9250 BMP-280

## Developer information
### Software
This is a PlatformIO project, developed using Atom editor (atom.io)-
To make it compile first run the following commands to install dependencies:
pio lib install "adafruit/Adafruit GFX Library"
pio lib install "adafruit/Adafruit BusIO"
...[TBD]

The complete code is auto formatted using "atom-beautify", which is installed like this:
apm install atom-beautify
Selected auto formatter is "uncrustify" with default settings.
   
### Hardware

**Schematic of the rotary button**
* https://easyeda.com/hogo20/CJMCU-111-rotary-encoder-schematic
* Solder a 330 Ohm (+-5%) from pin 1 to pin 4 of the CJMCU-111 rotary encoder switch. In other words: Lower the value of bottom most resistor. This way a button push can be detected bei the logic level input GPIO.

This hardware description comes from https://github.com/frankschoeniger/LIN_Interface as I found it very good and this project is slightly inspired by "LIN_Interface".

**LINBUS Breakout board**

|PIN LIN    |  PIN NodeMCU-ESP32 |
|-----------|--------------------|
| GND       |  GND |  
| CS        |  2                 |
| FAULT     |  VCC *+5V*  |
| TxD       |  3                 |
| RxD       |  4                 |
| VCC       |  Vcc *+5V* |

**Connector 2 on LIN Breakout:**

- UBat --> + Akku
- LIN  --> IBS PIN 2
- GND  --> GNC Connector Sensor not Akku !!


**Hella IBS 200X Sensor**

- PIN1: --> + Akku
- PIN2: --> LIN Bus


**Display**

|PIN Display  |  PIN Arduin Nano|
|---|----|
|LED          |  VCC +5V |
|SCK          |  13 |
|SDA          |  11 |
|A0 *(DC)*      |  9 |
|Reset RST    |  8 |
|CS           |  10 |
GND          |  GND
VCC          |  VCC +5V
