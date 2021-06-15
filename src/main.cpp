#include "header.h"

OLED brew_machine;

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);
	brew_machine.start();
}

// the loop function runs over and over again until power down or reset
void loop() {
	brew_machine.refresh();
	brew_machine.eventListener();
	yield();
}