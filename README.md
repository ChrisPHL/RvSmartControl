# RV Smart Control
RV Smart Control is a LIN-bus based control system for recreational vehicles. It runs on an ESP32 (NodeMCU-ESP32/ESP32 DEVKITV1), utilizes an extra *LINBUS Breakout - skpang.co.uk* (http://skpang.co.uk/catalog/linbus-breakout-board-p-1417.html) board and a *CJMCU-111* rotary input device. (I wrote an Arduino demo which shows the latter one interrupt driven.)
Furhtermore an *ILI9341 Display* (320x240 px) is used to show the menu driven control.


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
