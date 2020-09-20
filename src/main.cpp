#include "header.h"

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
	brew_machine.eventListener();
	yield();
}
