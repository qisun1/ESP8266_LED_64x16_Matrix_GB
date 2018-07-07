/*
    Name:       MsgBoard_ESP8266_GB.ino
    Created:	7/7/2018 8:36:29 AM
    Author:     Qi Sun
*/

#include <ESP8266_LED_64x16_Matrix_GB.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>


/************ connection pins ******************/
#define latchPin D1
#define clockPin D2
#define data_R1 D3
//const byte data_R2 = 11;
#define en_74138 D0
#define la_74138 D8
#define lb_74138 D7
#define lc_74138 D6
#define ld_74138 D5

/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) ******************/
#define DEVICENAME "msgboard_gb"
#define mywifi_ssid "xxxxxxxxxx"
#define mywifi_password "xxxxxxxxxx"
#define mymqtt_server "xxxxxxxxxxx"

#define mqtt_password  "xxxxxxxxxxxx"
#define mymqtt_port 1883

const char* topic_message = "message/" DEVICENAME;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

ESP8266_LED_64x16_Matrix_GB LEDMATRIX;

String message;
unsigned long message_timestamp;
unsigned long message_lifetime;

//0 for turn off
//1 for scroll horizontal
//2 for scroll vertical
uint8_t DisplayMode = 0;

void setup()
{
	Serial.begin(115200);

	mqttClient.setServer(mymqtt_server, mymqtt_port);
	mqttClient.setCallback(callback);

	setup_wifi();
	connect_mqtt();

	//I do not know why, this 4 seconds delay is necessary on my board to get things to work
	delay(3000);

	Serial.println("begin");
	uint8_t t[8] = { latchPin, clockPin, data_R1, en_74138, la_74138, lb_74138, lc_74138, ld_74138 };
	LEDMATRIX.setPins(t);

	//screen mode is an integer code: 0:64x16 1:128x16;
	LEDMATRIX.setDisplay(0, 1);

}

void loop()
{
	if ((millis() - message_timestamp) > message_lifetime)
	{
		LEDMATRIX.turnOff();
		DisplayMode = 0;
	}

	switch (DisplayMode)
	{
		case 1: 
			LEDMATRIX.scrollTextHorizontal(100);
			break;
		case 2:
			LEDMATRIX.scrollTextVertical(1000);
			break;
		case 3:
			LEDMATRIX.BreakTextInFrames(1000);
			break;
		default:
			delay(200);
			break;
	}

	if (WiFi.status() != WL_CONNECTED) {
		setup_wifi();
		return;
	}


	if (mqttClient.connected()) {
		mqttClient.loop();

	}
	else
	{
		connect_mqtt();
		return;
	}
}

boolean setup_wifi() {

	delay(10);
	// We start by connecting to a WiFi network
	Serial.println("Connecting to WiFi");

	WiFi.begin(mywifi_ssid, mywifi_password);

	int mycount = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		//Serial.print(".");
		mycount++;
		if (mycount > 20)
		{
			return false;
		}
	}
	Serial.println("Connected");
	return true;
}

void connect_mqtt() {
	int t = 0;
	while (!mqttClient.connected()) {
		Serial.print("Connecting to MQTT ...");
		t++;
		if (t > 3)
		{
			return;
		}
		if (mqttClient.connect(DEVICENAME, DEVICENAME, mqtt_password)) {
			if (mqttClient.subscribe(topic_message))
			{
				Serial.println("subscribe to ");
				Serial.println(topic_message);
			}
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(mqttClient.state());
			Serial.println(". try again in 2 seconds");
			// Wait 2 seconds before retrying
			delay(2000);
		}
	}
}

void callback(char* topic, byte* payload, unsigned int length) {

	// first byte in message is the dispaly mode 
	// the following 4 bytes are display time
	// the rest bytes are data, two byte per character. first byte 0xaa are ascii characters.
	// for ascii chr, the value is ascii-32
	// all two-byte unit not starting with 0xaa, are 2-byte gb2312 encoding. 

	Serial.println("Message arrived");

	if (length<6)
	{
		return;
	}
	char mode;
	message = "";

	LEDMATRIX.turnOff();
	Serial.println("receiving message");

	mode = (char)payload[0];

	String tt = "";
	for (uint8_t i = 1; i < 5; i++) {
		tt += (char)payload[i];
	}

	for (uint8_t i = 5; i < length; i++) {
		message += (char)payload[i];
	}

	message_timestamp = millis();
	message_lifetime = ((unsigned long)tt.toInt()) * 1000;

	LEDMATRIX.setMessage(message);

	switch (mode)
	{
	case 'm':
		DisplayMode = 1;
		break;
	case 'v':
		DisplayMode = 2;
		break;
	case 's':
		DisplayMode = 3;
		break;
	default:
		break;
	}

	if (message_lifetime == 0)
	{
		return;
	}
	delay(100);
	LEDMATRIX.turnOn();

}