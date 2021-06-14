#include "gcp.h"

GCP::GCP() {
	GCP(targetTemp = DEFAULT_BREW_TEMP, tempOffset = 0.0);
}

GCP::GCP(double targetTemp, double offset) {
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);
	this->setTempOffset(offset);
	this->setMode(brew);
	this->actualTemp = this->getActualTemp();
}

GCP::~GCP() {
	double targetTemp = this->getTargetTemp();
	if (targetTemp > maxBrewTemp) targetTemp = maxBrewTemp;
};

mode GCP::getCurrentMode() {
	return this->currentMode;
}

void GCP::setMode(mode currentMode){
	this->currentMode = currentMode;
}

void GCP::setTargetTemp(double temp) {
	if (temp < minBrewTemp) temp = minBrewTemp;
	else if (temp > maxBrewTemp) temp = maxBrewTemp;
	this->targetTemp = temp;
}

void GCP::setTargetSteamTemp(double temp) {
	if (temp < minSteamTemp) temp = minSteamTemp;
	else if (temp > maxSteamTemp) temp = maxSteamTemp;
	this->targetSteamTemp = temp;
}

void GCP::incrementTemp() {
	double *temp;
	if(this->currentMode == steam) temp = &targetSteamTemp;
	else temp = &targetTemp;
	*temp += 0.5;
}

void GCP::decrementTemp() {
	double *temp;
	if(this->currentMode == steam) temp = &targetSteamTemp;
	else temp = &targetTemp;
	*temp -= 0.5;
}

double GCP::getTargetTemp() {
	return this->targetTemp;
}

double GCP::getTargetSteamTemp() {
	return this->targetSteamTemp;
}

double GCP::getActualTemp() {
	double temp =  tempProbe.temperature(100, RREF);
	uint8_t fault = tempProbe.readFault();
	if (fault) {
		Serial.print("Fault 0x"); Serial.println(fault, HEX);
		if (fault & MAX31865_FAULT_HIGHTHRESH) Serial.println("RTD High Threshold");
		if (fault & MAX31865_FAULT_LOWTHRESH) Serial.println("RTD Low Threshold"); 
		if (fault & MAX31865_FAULT_REFINLOW) Serial.println("REFIN- > 0.85 x Bias");
		if (fault & MAX31865_FAULT_REFINHIGH) Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
		if (fault & MAX31865_FAULT_RTDINLOW) Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
		if (fault & MAX31865_FAULT_OVUV) Serial.println("Under/Over voltage");
		tempProbe.clearFault();
	}
	this->actualTemp = temp;
	return temp;
}

double GCP::getTempOffset(){
	return this->tempOffset;
}

void GCP::setTempOffset(double offset){
	this->tempOffset = offset;
}

void GCP::update() {
	double targetTemp = this->getTargetTemp();
	double targetSteamTemp = this->getTargetSteamTemp();
	double actualTemp = this->getActualTemp();
	double offset = this->getTempOffset();

	brewTempManager.compute();
	steamTempManager.compute();

	Serial.printf("\nActual Temp: %f\n", actualTemp);
	Serial.printf("Brew  Temp: %f %f\n", targetTemp, brew_output);
	Serial.printf("Steam Temp: %f %f\n", targetSteamTemp, steam_output);
	Serial.printf("Offset: %f\n", offset);
	/* 
		Brew Relay and Steam Relay will always be calculating
		When power switch is on the heater will heat until it gets to targetBrewtemp
		Brew Lamp will turn on once the brew relay is off (once target temp is reached)

		If steam switch is pushed then the brew lamp is turned off
		Power defaults to steam relay, brew relay is off
		Steam lamp turn on when steam relay is off

		This is all handled on the hardware side.

		If temperature rises above maximum safe temperature turn off relay
	*/

	if(millis() - cycleStartTime > cycleRunTime) {
		cycleStartTime += cycleRunTime;
	}
	
	if(brew_output < millis() - cycleStartTime) digitalWrite(HEATER_PIN, ON);
	else digitalWrite(HEATER_PIN, OFF);

	if(steam_output < millis() - cycleStartTime) digitalWrite(STEAM_PIN, ON);
	else digitalWrite(STEAM_PIN, OFF);

	if(actualTemp >= EMERGENCY_SHUTOFF_TEMP) {
		digitalWrite(STEAM_PIN, OFF);
		digitalWrite(HEATER_PIN, OFF);
	}
}