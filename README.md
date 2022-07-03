# RV Smart Control
RV Smart Control is a LIN-bus based control system for recreational vehicles. It runs on an ESP32 (NodeMCU-ESP32/ESP32 DEVKITV1), utilizes an extra LINBUS Breakout board and a CJMCU-111 rotary input device. (I wrote an [Arduino demo which shows the latter one interrupt driven](https://github.com/ChrisPHL/CJMCU-111-rotary-button-demo).) Furhtermore an ILI9341 Display (320x240 px) is used to show the menu driven control and a combined sensor board is connected to aquire environmental information.

## Part List
Please note: The links are just to showcase the components. You may buy your parts elsewhere, of course.
* NodeMCU-ESP32 - https://joy-it.net/de/products/SBC-NodeMCU-ESP32
* LIN-Bus breakout board - https://buyzero.de/products/lin-bus-breakout-board 
* USB Step Down DC-DC Spannungswandler 6-24V zu 5V 3A Arduino Spannungsregler - https://www.makershop.de/module/step-downup/6-24v-usb-spannungswandler/
* 2.8 Inch ILI9341 240x320 SPI TFT LCD Display Touch Panel SPI Serial Port Module - http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807
* CJMCU-111 DC 5V Push Button Switch Rotary Encoder / Momentary Push Switch - https://funduinoshop.com/elektronische-module/keypads-und-buttons/rotary-encoder/rotary-encoder-cjmcu-111
* GY-91 - https://artofcircuits.com/product 10dof-gy-91-4-in-1-mpu-9250-and-bmp280-multi-sensor-module


## Software
This is a PlatformIO project, developed using VSCodium editor with PlatformIO IDE extension. To install PlatformIO IDE extension you might need to activate the M$ extension maketplace for VSCode This procedure is described here: https://github.com/VSCodium/vscodium/blob/master/DOCS.md#how-to-use-the-vs-code-marketplace.

### Prerequisits
Prior to compile you need to run once the following commands to install dependencies:
* pio pkg install -l "adafruit/Adafruit ILI9341"
* pio pkg install -l "adafruit/Adafruit BMP280 Library"
* pio pkg install -l "hideakitai/MPU9250"
* pio pkg install -l "adafruit/Adafruit BusIO"

The complete code is auto formatted using "atom-beutify", which is installed like this:
apm install atom-beautify
Selected auto formatter is "uncrustify" with default settings.

### 3rd Party Libraries
RvSmartControl uses the [RemoteDebug library](https://github.com/JoaoLopesF/RemoteDebug) make serial debugging obsolete and uses a websocket over WiFi instead. On the remote side you can use [RemoteDebugApp](https://github.com/JoaoLopesF/RemoteDebugApp) to receive debug messages and even control the debugging behaviour.

## Hardware
This hardware description comes from https://github.com/frankschoeniger/LIN_Interface as I found it very good and the this project is slightly inspired by "LIN_interface". This description uses an Arduino Nano running the code.

### Wiring Of The Components
TODO: Create Fritzing project to illustrate the wiring.

*LINBUS Breakout - skpang.co.uk*

|  PIN LIN  |  PIN NodeMCU-ESP32  |
|-----------|---------------------|
|  GND      |  GND                |  
|  CS       |  D12                |
|  FAULT    |  not connected      |
|  TxD      |  TX2                |
|  RxD      |  RX2                |
|  VCC      |  D14                |


*Connector 2 on LIN Breakout*

|  PIN LIN  |  PIN IBS / Car        |
|-----------|-----------------------|
|  UBat     |  Batttery +12V        |  
|  LIN      |  IBS PIN 2            |
|  GND      |  GND terminal of IBS  |


*Hella IBS 200X Sensor*

|  IBS      |  PIN IBS / LINBUS breakout  |
|-----------|-----------------------------|
|  PIN1     |  Batttery +12V              |  
|  PIN2     |  LIN                        |


*ILI9341 TFT Display*

|  PIN Display  |  PIN NodeMCU-ESP32  |
|---------------|---------------------|
|  VCC          |  D27                |
|  GND          |  GND                |
|  CS           |  D32                |
|  RESET        |  D25                |
|  D/C          |  D33                |
|  SDI(MOSI)    |  D23                |
|  SCK          |  D18                |
|  LED          |  D26                |
|  SDO(MOSI)    |  D19                |


 *GY-91 Sensor*

|  PIN GY-91  |  PIN NodeMCU-ESP32  |
|-------------|---------------------|
|  VIN        |  not connected      |
|  3V3        |  D13                |
|  GND        |  GND                |
|  SCL        |  D22                |
|  SDA        |  D21                |
|  SDD/SAO    |  D4                 |
|  NCS        |  not connected      |
|  CSB        |  not connected      |



# Things to do
There are allways things that one want to see included into this project. So for a long term plan I put some use cases to remember here: 
* Control [Truma Combi heating devices](https://www.truma.com/int/en/products/truma-heater)
* Integrate [DalyBMS UART Protocol](https://github.com/maland16/daly-bms-uart) over Serial2
* Water level display utilizing the GY-91 multi sensor.

If you have done some work on that allready or if you just want to contribute to the project in any way, feel free to file a pull request or open an issue. 