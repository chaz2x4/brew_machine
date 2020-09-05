#include "header.h"

#define RTD_SENSOR_PIN 4
#define BREW_LIGHT_PIN 13
#define STEAM_LIGHT_PIN 12
#define RELAY_PIN 27

OLED brew_machine;

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(9600);
	brew_machine.start();
	//Secondary Mission: connect to coffee server
}

// the loop function runs over and over again until power down or reset
void loop() {
	brew_machine.refresh();
	delay(250);
	yield();
}
