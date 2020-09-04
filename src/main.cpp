#include "header.h"

#define RTD_SENSOR_PIN 4
#define BREW_LIGHT_PIN 13
#define STEAM_LIGHT_PIN 12
#define RELAY_PIN 27

GCP brew_machine;

void runTest();
void printEEPROMVal();

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(9600);
	//Secondary Mission: connect to coffee server
}

// the loop function runs over and over again until power down or reset
void loop() {

	runTest();

	brew_machine.refresh();
	delay(250);
}

void runTest() {
	//Check status of lights
	bool brewStatus = brew_machine.isBrewReady();
	bool steamStatus = brew_machine.isSteamReady();
	bool powerStatus = brew_machine.isPowerOn();
	Serial.print("Brew status: ");
	Serial.println(brewStatus);
	Serial.print("Steam status: ");
	Serial.println(steamStatus);
	Serial.print("Power status: ");
	Serial.println(powerStatus);

	float targetTemp = brew_machine.getTargetTemp();
	Serial.print("Target Temp: ");
	Serial.println(targetTemp);

	float actualTemp = brew_machine.getActualTemp();
	Serial.print("Actual Temp: ");
	Serial.println(actualTemp);

	printEEPROMVal();
	for (int i = 0; i < 5; i++) {
		brew_machine.decrementTemp();
	}
	printEEPROMVal();
}

void printEEPROMVal() {
	Serial.print("EEPROM value: ");
	float f = 0.00f;
	EEPROM.get(0, f);
	Serial.println(f, 2);
}
