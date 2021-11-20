#include "GCP.h"

void GCP::init() {
	init(targetTemp = DEFAULT_BREW_TEMP, targetSteamTemp = DEFAULT_STEAM_TEMP, tempOffset = DEFAULT_OFFSET);
}

void GCP::init(double targetTemp, double targetSteamTemp, double offset) {
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);
	this->setTargetSteamTemp(targetSteamTemp);
	this->setTempOffset(offset);
	this->setMode(brew);
	this->actualTemp = this->getActualTemp();
	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);
	brewTempManager.tune(30.0, 0.6, 140.0);
	steamTempManager.tune(90.0, 1.2, 140.0);
}

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
	double temp;
	if(this->currentMode == steam) temp = targetSteamTemp;
	else temp = targetTemp;
	temp += 0.5;
	if(this->currentMode == steam) this->setTargetSteamTemp(temp);
	else this->setTargetTemp(temp);
}

void GCP::decrementTemp() {
	double temp;
	if(this->currentMode == steam) temp = targetSteamTemp;
	else temp = targetTemp;
	temp -= 0.5;
	if(this->currentMode == steam) this->setTargetSteamTemp(temp);
	else this->setTargetTemp(temp);
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
	temp += tempOffset;
	this->actualTemp = temp;
	return temp;
}

double GCP::getTempOffset(){
	return this->tempOffset;
}

void GCP::setTempOffset(double offset){
	if(offset > MAX_OFFSET) this->tempOffset = MAX_OFFSET;
	else if(offset < MIN_OFFSET) this->tempOffset = MIN_OFFSET;
	else this->tempOffset = offset;
}

double GCP::getBrewOutput(){
	return this->brew_output;
}

double GCP::getSteamOutput(){
	return this->steam_output;
}

double* GCP::getTunings(double* tunings){
	if(this->currentMode == steam) return steamTempManager.getTunings(tunings);
	else return brewTempManager.getTunings(tunings);
}

void GCP::setTunings(double kp, double ki, double kd){
	if(this->currentMode == steam) steamTempManager.tune(kp, ki, kd);
	else brewTempManager.tune(kp, ki, kd);
}

void GCP::update() {
	brewTempManager.compute();
	steamTempManager.compute();

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
	
	if(brew_output < millis() - cycleStartTime) digitalWrite(HEATER_PIN, OFF);
	else digitalWrite(HEATER_PIN, ON);

	if(steam_output < millis() - cycleStartTime) digitalWrite(STEAM_PIN, OFF);
	else digitalWrite(STEAM_PIN, ON);
	
	double actualTemp = this->getActualTemp();
	if((actualTemp + tempOffset) >= EMERGENCY_SHUTOFF_TEMP) {
		digitalWrite(STEAM_PIN, OFF);
		digitalWrite(HEATER_PIN, OFF);
	}
}