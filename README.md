# RV Smart Control
RV Smart Control is a LIN-bus based control system for recreational vehicles. By now it runs on an Arduino Nano, utilizes an extra *LINBUS Breakout - skpang.co.uk* board and a *CJMCU-111* rotary input device. (I wrote an Arduino demo which shows the latter one interrupt driven.) Furhtermore a *ST7735 Display* is used to show the menu driven control.
Later on there will be a port to a NodeMCU which runs an Espressif Esp8266 controller that is much faster and got way more flash memory connected.

## Developer information
### Software
This is a PlatformIO project, developed using Atom editor (atom.io)-
To make it compile first run the following commands to install dependencies:
pio lib install "adafruit/Adafruit GFX Library"
pio lib install "adafruit/Adafruit BusIO"
pio lib install "adafruit/Adafruit ST7735 and ST7789 Library"
pio lib install "rocketscream/Low-Power"

The complete code is auto formatted using "atom-beutify", which is installed like this:
apm install atom-beautify
Selected auto formatter is "uncrustify" with default settings.
   
### Hardware
This hardware description comes from https://github.com/frankschoeniger/LIN_Interface as I found it very good and the this project is slightly inspired by "LIN_interface". This description uses an Arduino Nano running the code.

**LINBUS Breakout - skpang.co.uk**

|PIN LIN    |   PIN Arduino Nano |
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


**ST7735S Display:**

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
