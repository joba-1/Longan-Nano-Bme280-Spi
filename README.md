# Longan Nano BME280 SPI Example

This is the result of me trying to understand longan nano spi used with a sensor.

## Topics
It contains various examples that should be reusable in other code
* Serial output with printf()
* Using RGB LED of Longan Nano
* Read onchip temperature and reference voltage
* Init SPI
* Use BME280 sensor driver from Bosch with SPI
* Use 160x80 OLED SPI LCD 

## Image
![image](https://user-images.githubusercontent.com/32450554/94979471-1e94e200-0523-11eb-826d-37fe5138cc62.png)

## Connections
Nano        | BME280
------------|-------
3V3         | Vcc
G           | Gnd
A4 (NSS0)   | CSE
A5 (SCK0)   | SCL
A6 (MISO0)  | SDO
A7 (MOSI0)  | SDA

Nano        | USB2Serial | Comment
------------|------------|--------
3V3         | Vcc        | not needed if powered via USB or other
Gnd         | Gnd
A9 (T0)     | Rx
A10 (R0)    | Tx         | not needed if flashing via USB/DFU or JTAG

## 
* Author  Joachim Banzhaf
* License Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)

why do you [find valuable resources](https://github.com/MuellerA/LonganNanoTest/tree/master/SpiDma/src) only after it is too late. Ok I2C will be easier now :)
