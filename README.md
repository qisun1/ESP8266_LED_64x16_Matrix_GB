# ESP8266 libary to drive LED 64x16 or 64x32 (RED/GREEN) matrix, supporting Chinese GB2312 encoding
Make sure that you upload the font file HZK16 into the SPIFFS before using this library. You can use Arduino IDE to do this. The instruction is at http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html. 
This library provides an ESP8266 driver to run LED matrix, supporting both ASCII encoding and GB. Before loading your sketch, you need to load the font file into ESP8266 file system SPIFFs. The font file is in "data" directory. Here is the instruction how: http://www.instructables.com/id/Using-ESP8266-SPIFFS/. 

I am still testing it. It should work with one or multiple 64x16 LED matrix (red). If you have red-green matrix, the code needs to be slightly modified if you plan to use the green color. My LED matrxi has following pins: enable, clock, latch, data_r1, data r2(for 64x32 matrix), data_g1, data_g2 (for green color), and 4 row pins labeled (a, b, c, d). 

I directly connect pins of the nodemcu to the LED matrix. I do not use level shift between the board and the LED matrix. 

The input string must use two bytes for each character. For GB2312 encoded character, use the GB2312 bytes. For ASCII character, first byte should be "0xaa", the second byte is the ascii code.

## Examples

In examples directory.

## Limitations
The current code does not yet implement pins for green color as well as data_r2 pin used for 64x32 matrix. Slight modification is needed. I am probably not going to do that as I only have red color matrix. After this project, I am going to switch RGB LED matrix.



## Compatible Hardware

The library works with ESP8266 based board, and one or multiple 64x16 LED matrix.


## License

This code is released under the MIT License.
