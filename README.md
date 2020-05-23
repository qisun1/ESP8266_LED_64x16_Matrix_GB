# ESP8266 libary to drive LED 64x16 or 64x32 (RED/GREEN) matrix, supporting Chinese GB2312 encoding
Before using this library, make sure that you upload the font file HZK16 into the SPIFFS. A copy of the HZK file is included in the "data" directory. You can use Arduino IDE for uploading. The instruction is at https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html.

This library provides an ESP8266 driver to run LED matrix, supporting mixed ASCII and GB encodings. The input string must use two bytes for each character. For GB2312 encoded character, use the two GB2312 bytes. For ASCII character, first byte should be "0xaa", the second byte is the actual ascii code.

For pin connections, please refer to this page: https://github.com/qisun1/ESP8266_LED_64x16_Matrix

In your sketch, you might need to add "delay(4000)" at the beginning of "setup". It is needed on my sketch, otherwise it would cause reset once timer1 is enabled. I do not know why, maybe the startup series of ESP8266 has conflict with the timer1.

## Examples

In examples directory.

## Limitations
See https://github.com/qisun1/ESP8266_LED_64x16_Matrix


## Compatible Hardware

The library works with ESP8266 based board, and one or multiple 64x16 LED matrix.


## License

This code is released under the MIT License.
