/*
  ESP8266_LED_64x16_Matrix.cpp - A 64x16 matrix driver for MQTT.
  Qi Sun
  https://github.com
*/

#include "ESP8266_LED_64x16_Matrix_GB.h"
#include "Arduino.h"


ESP8266_LED_64x16_Matrix_GB::ESP8266_LED_64x16_Matrix_GB()
{
}

//screen mode is an integer code: 0:64x16 1:128x16;
void ESP8266_LED_64x16_Matrix_GB::setDisplay(uint8_t matrixType, uint8_t panels)
{
	SPIFFS.begin();
	ESP8266_LED_64x16_Matrix_GB::isrInstance = this;
	switch (matrixType)
	{
		case 0:
			columnNumber = 8 * panels;
			rowCount = 16;
			bufferSize = (columnNumber+1)*rowCount;
		break;
		default:
			columnNumber = 8 * panels;
			rowCount = 16;
			bufferSize = (columnNumber + 1)*rowCount;
			break;
	}

	buffer = new uint8_t[bufferSize*2];

	scrollPointer = 0;
	scanRow = 0;
	clear_buffer();

	timer1_attachInterrupt(interruptHandler);


	timer1_write(nextT);
	timer1_disable();


	delay(100);
}

void ESP8266_LED_64x16_Matrix_GB::setPins(uint8_t pins[8])
{
	latchPin = pins[0];
	clockPin = pins[1];
	data_R1 = pins[2];
	//data_R2 = ;
	en_74138 = pins[3];
	la_74138 = pins[4];
	lb_74138 = pins[5];
	lc_74138 = pins[6];
	ld_74138 = pins[7];

	rowPin = (1 << la_74138) | (1 << lb_74138) | (1 << lc_74138) | (1 << ld_74138);

	pinMode(latchPin, OUTPUT);  pinMode(clockPin, OUTPUT);
	pinMode(data_R1, OUTPUT);   //pinMode(data_R2, OUTPUT);

	pinMode(en_74138, OUTPUT);
	pinMode(la_74138, OUTPUT);  pinMode(lb_74138, OUTPUT);
	pinMode(lc_74138, OUTPUT);  pinMode(ld_74138, OUTPUT);

}


//expected inPut message is GB encoded characters, two types for each character
//If you want to display ascii character with one column, you still need to use two type,
//The first byte needs to be 0xaa, the second byte is the the ascii code
//0xaa is not used in qu code of GB2312
void ESP8266_LED_64x16_Matrix_GB::setMessage(String inputMessage)
{
	char extraFontPointer = 0;
	for (uint8_t i = 0; i < inputMessage.length(); i = i + 2) {
		char t = (char)inputMessage[i];
		if (t == 0xaa)
		{
			char t2 = (char)(inputMessage[i + 1] - (char)32);
			if (t2 <= 95)
			{
				message += t2;
			}
		}
		else if (extraFontPointer < extraFontTotal)
		{
			//get font and insert index
			//each font used two columns on the LED matrix, fontPart1 and fontPart2 are for the two columns
			char gbcode[2];
			char fontPart1[16];
			char fontPart2[16];
			gbcode[0] = (char)inputMessage[i];
			gbcode[1] = (char)inputMessage[i + 1];
			gbFont(gbcode, fontPart1, fontPart2);

			memcpy(font8x16_extra + (extraFontPointer * 16), fontPart1, 16);
			char p = char(extraFontPointer + extraFontStart);
			message += p;
			extraFontPointer++;
			memcpy(font8x16_extra + (extraFontPointer * 16), fontPart2, 16);
			p = char(extraFontPointer + extraFontStart);
			message += p;
			extraFontPointer++;
		}

	}
}

void ESP8266_LED_64x16_Matrix_GB::gbFont(char* in, char* out1, char* out2)
{
	uint32 qu = in[0] - (byte)0xa0;
	uint32 wei = in[1] - (byte)0xa0;
	uint32 offset = ((94 * (qu - 1) + wei - 1)) * 32;

	//Serial.print("offset: ");
	//Serial.println(offset);
	if (offset >= 267584)
	{
		offset = 6048;
	}

	File f = SPIFFS.open("/HZK16", "r");
	if (!f) {
		Serial.println("file open failed");
	}

	f.seek(offset, SeekSet);
	char out[32];
	f.readBytes(out, 32);

	f.close();

	for (uint8 i = 0; i < 16; i++)
	{
		out1[i] = out[2 * i];
		out2[i] = out[2 * i + 1];
	}


}



void ESP8266_LED_64x16_Matrix_GB::turnOn()
{
	//timer1_isr_init();
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
}

void ESP8266_LED_64x16_Matrix_GB::turnOff()
{
	timer1_disable();
	digitalWrite(en_74138, HIGH);

	clear_buffer();
	scanRow = 0;
	scrollPointer = 0;

}


void ESP8266_LED_64x16_Matrix_GB::clear_buffer()
{
	for (uint16_t i = 0; i <bufferSize; i++)
	{
		buffer[i] = 0x00;
	}
}


void ESP8266_LED_64x16_Matrix_GB::drawChar(uint16_t pixel_x, uint16_t pixel_y, uint8_t n) {
	uint8_t fontrows= 16;
	uint16_t index;
	uint8_t charbytes[fontrows];

	if (n >= (extraFontStart + extraFontTotal))
	{
		return;
	}
	else if (n < extraFontStart)
	{
		index = (n)*fontrows; // go to the right code for this character																						 // addressing start at buffer and add y (rows) * (WIDTH is 64 so WIDTH/8) is 8 plus (x / 8) is 0 to 7
		for (uint8_t i = 0; i<fontrows; i++) {  // fill up the charbytes array with the right bits
			charbytes[i] = font8x16_basic[index + i];
		};
	}
	else
	{
		index = (n - extraFontStart)*fontrows; // go to the right code for this character																						 // addressing start at buffer and add y (rows) * (WIDTH is 64 so WIDTH/8) is 8 plus (x / 8) is 0 to 7
		for (uint8_t i = 0; i<fontrows; i++) {  // fill up the charbytes array with the right bits
			charbytes[i] = font8x16_extra[index + i];
		};
	}
	
																				 // addressing start at buffer and add y (rows) * (WIDTH is 64 so WIDTH/8) is 8 plus (x / 8) is 0 to 7
	uint8_t *pDst = buffer + (pixel_y * (columnNumber + 1)) + pixel_x;


	uint8_t *pSrc = charbytes; // point at the first set of 8 pixels    
	for (uint8_t i = 0; i<fontrows; i++) {
		*pDst = *pSrc;     // populate the destination byte
		pDst += columnNumber + 1;         // go to next row on buffer
		pSrc++;            // go to next set of 8 pixels in character
	}

	//Serial.println(buffer[0]);
	//Serial.println("draw buffer111");
};


void ESP8266_LED_64x16_Matrix_GB::moveLeft(uint8_t pixels, uint8_t rowstart, uint8_t rowstop) { // routine to move certain rows on the screen "pixels" pixels to the left
	uint8_t row, column;
	uint16_t index;
	for (column = 0; column<(columnNumber + 1); column++) {
		for (row = rowstart; row<rowstop; row++) {
			index = (row * (columnNumber + 1)) + column; /// right here!
			if (column == (columnNumber))
				buffer[index] = buffer[index] << pixels; // shuffle pixels left on last column and fill with a blank
			else {                // shuffle pixels left and add leftmost pixels from next column
				uint8_t incomingchar = buffer[index + 1];
				buffer[index] = buffer[index] << pixels;
				for (uint8_t x = 0; x<pixels; x++) { buffer[index] += ((incomingchar & (0x80 >> x)) >> (7 - x)) << (pixels - x - 1); };
			}
		}
	}
};

void ESP8266_LED_64x16_Matrix_GB::scrollTextHorizontal(uint16_t delaytime)
{
	// display next character of message
	drawChar(columnNumber, 0, message[scrollPointer % (message.length())]);
	scrollPointer++;
	if (scrollPointer >= message.length())
	{
		scrollPointer = 0;
	}
	// move the text 1 pixel at a time
	for (uint8_t i = 0; i<8; i++) {
		delay(delaytime);
		moveLeft(1, 0, rowCount);

	};

}

void ESP8266_LED_64x16_Matrix_GB::BreakTextInFrames(uint16_t delaytime)
{
	clear_buffer();
	for (uint8_t i = 0; i < columnNumber; i++)
	{
		drawChar(i, 0, message[scrollPointer]);
		scrollPointer++;
		if (scrollPointer >= message.length())
		{
			scrollPointer = 0;
			break;
		}
	}
	delay(delaytime);
}


//not tested , need to double the buffer size in the setScreen
void ESP8266_LED_64x16_Matrix_GB::scrollTextVertical(uint16_t delaytime)
{
	for (uint16_t i = bufferSize; i <(2* bufferSize); i++)
	{
		buffer[i] = 0x00;
	}
	for (uint8_t i = 0; i < columnNumber; i++)
	{
		drawChar(i, rowCount, message[scrollPointer]);
		scrollPointer++;
		if (scrollPointer >= message.length())
		{
			scrollPointer = 0;
			break;
		}
	}

	for (uint8_t t = 0; t < rowCount; t++)
	{
		for (uint8_t i = 0; i< (rowCount * 2 - 1); i++)
		{
			memcpy(buffer + i * (columnNumber + 1), buffer + (i + 1)*(columnNumber + 1), columnNumber + 1);
		}
		delay(50);
	}
	delay(delaytime);

}

void  ESP8266_LED_64x16_Matrix_GB::ISR_TIMER_SCAN()
{
	//noInterrupts();
	digitalWrite(en_74138, HIGH);     // Turn off display
									  // Shift out 8 columns
	for (uint8_t column = 0; column<columnNumber; column++) {
		uint8_t index = column + (scanRow *(columnNumber+1));
		shiftOut(data_R1, clockPin, MSBFIRST, buffer[index]);
	};

	digitalWrite(latchPin, LOW);
	digitalWrite(latchPin, HIGH);

	WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 8, rowPin);
	uint32_t rowPinSet = ((scanRow >> 3) & 0x01) << ld_74138;
	rowPinSet = rowPinSet | (((scanRow >> 2) & 0x01) << lc_74138);
	rowPinSet = rowPinSet | (((scanRow >> 1) & 0x01) << lb_74138);
	rowPinSet = rowPinSet | ((scanRow & 0x01) << la_74138);
	WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + 4, rowPinSet);
	digitalWrite(en_74138, LOW);     // Turn on display
	scanRow++; 
	// Do the next pair of rows next time this routine is called
	if (scanRow == rowCount)
	{
		scanRow = 0;
		
	}
	timer1_write(nextT);
	//interrupts();
}

ESP8266_LED_64x16_Matrix_GB * ESP8266_LED_64x16_Matrix_GB::isrInstance;

void ESP8266_LED_64x16_Matrix_GB::interruptHandler()
{
	isrInstance->ISR_TIMER_SCAN();
}
