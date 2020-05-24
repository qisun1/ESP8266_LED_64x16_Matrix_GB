/*
    Name:       MsgBoard_ESP8266_GB.ino
    Created:	7/7/2018 8:36:29 AM
    Author:     Qi Sun
*/

#include <ESP8266_LED_64x16_Matrix_GB.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Connections to board
// latchPin must be D0
#define latchPin D0

//The following pin can be changed if needed
#define clockPin D1
#define data_R1 D3
#define data_R2 D2
#define en_74138 D4
#define la_74138 D8
#define lb_74138 D7
#define lc_74138 D6
#define ld_74138 D5 

/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) ******************/
#define DEVICENAME "msgboard_gb"
#define mywifi_ssid "xxxxxxxxxxx"
#define mywifi_password "xxxxxxxxxxxxx"
#define mymqtt_server "xxxxxxxxxxxx"

#define mqtt_password  "xxxxxxxxxxx"
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

uint8_t Operation = 0; 
//0 turn of wifi to receive message; 1 to turn off wifi and display message
//with wifi on,  display not stable

void setup()
{
	WiFi.mode(WIFI_OFF);
	Serial.begin(115200);

	//I do not know why, this 4 seconds delay is necessary on my board to get things to work
	delay(2000);

	//set mqtt
	Serial.println("set mqtt server");
	mqttClient.setServer(mymqtt_server, mymqtt_port);
	mqttClient.setCallback(callback);
	Serial.println("mqtt set");

	Serial.println("set up the display");
	uint8_t t[8] = { latchPin, clockPin, data_R1, en_74138, la_74138, lb_74138, lc_74138, ld_74138 };
	LEDMATRIX.setPins(t);
	//screen mode 0: 0:64x16 single color red
	//number of panels: 1
	LEDMATRIX.setDisplay(0, 1);
	Serial.println("display_set");

	Operation == 0;
	WiFi.mode(WIFI_STA);
	setup_wifi();
	connect_mqtt();
	Serial.println("all, set, waiting for msg .... ");
}

void loop()
{
	if (Operation == 0)
	{
		if (WiFi.status() != WL_CONNECTED) {
			setup_wifi();
			return;
		}
		if (mqttClient.connected()) {
			mqttClient.loop();
			if (Operation == 1)
			{
				mqttClient.disconnect();
				WiFi.disconnect();
				WiFi.mode(WIFI_OFF);
				Serial.println("wifi off");
				LEDMATRIX.setMessage(message);
				LEDMATRIX.turnOn();
				Serial.println("display on");
			}
		}
		else
		{
			connect_mqtt();
			return;
		}
	}
	else
	{
		if ((millis() - message_timestamp) > message_lifetime)
		{
			Serial.println("time is up, turn off display");
			LEDMATRIX.turnOff();

			Serial.println("turn on wifi");
			WiFi.mode(WIFI_STA);
			setup_wifi();
			connect_mqtt();
			Serial.println("all set, waiting for msg .... ");
			Operation = 0;
		}
		else
		{
			LEDMATRIX.scrollTextHorizontal(100);
		}
		
	}
}

void callback(char* topic, byte* payload, unsigned int length) {

	// first byte in message is the dispaly mode 
	// the following 4 bytes are display time
	// the rest bytes are data, two byte per character. first byte 0xaa are ascii characters.
	// for ascii chr, the value is ascii-32
	// all two-byte unit not starting with 0xaa, are 2-byte gb2312 encoding. 
	if (length < 6)
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
	if (message_lifetime == 0)
	{
		return;
	}
	Operation = 1;
	Serial.println("message received, ready to display");
	Serial.println("display time " + message_lifetime);
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