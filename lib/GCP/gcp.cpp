#include "gcp.h"

GCP::GCP() {
	GCP(targetTemp = DEFAULT_BREW_TEMP);
}



GCP::GCP(double targetTemp) {
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);
	this->actualTemp = this->getActualTemp();
}

GCP::~GCP() {
	double targetTemp = this->getTargetTemp();
	if (targetTemp > maxBrewTemp) targetTemp = maxBrewTemp;
};

void GCP::setTargetTemp(double temp) {
	if (temp < minBrewTemp) temp = minBrewTemp;
	else if (temp > maxBrewTemp) temp = maxBrewTemp;
	this->targetTemp = temp;
}

void GCP::incrementTemp() {
	double temp = this->getTargetTemp();
	this->setTargetTemp(temp + 0.5);
}

void GCP::decrementTemp() {
	double temp = this->getTargetTemp();
	this->setTargetTemp(temp - 0.5);
}

double GCP::getTargetTemp() {
	return this->targetTemp;
}

double GCP::getPX() {
	return this->pressure;
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
	return temp;
}

void GCP::update() {
	double targetTemp = this->getTargetTemp();
	double actualTemp = this->getActualTemp();

	brewTempManager.compute();
	steamTempManager.compute();

	Serial.printf("\nActual Temp: %f\n", actualTemp);
	Serial.printf("Brew  Temp: %f %f\n", targetTemp, brew_output);
	Serial.printf("Steam Temp: %f %f\n", targetSteamTemp, steam_output);
	/* 
		Brew Relay and Steam Relay will always be calculating
		When power switch is on the heater will heat until it gets to targetBrewtemp
		Brew Lamp will turn on once the brew relay is off (once target temp is reached)

		If steam switch is pushed then the brew lamp is turned off
		Power defaults to steam relay, brew relay is off
		Steam lamp turn on when steam relay is off

		This is all handled on the hardware side.

		If temperature rises above maixmum safe temperature (10C above steam temp) turn off relay

	*/

	if(millis() - cycleStartTime > cycleRunTime) {
		cycleStartTime += cycleRunTime;
	}
	
	if(brew_output < millis() - cycleStartTime) digitalWrite(HEATER_PIN, ON);
	else digitalWrite(HEATER_PIN, OFF);

	if(steam_output < millis() - cycleStartTime) digitalWrite(STEAM_PIN, ON);
	else digitalWrite(STEAM_PIN, OFF);

	if(actualTemp >= emergencyShutoffTemp) {
		digitalWrite(STEAM_PIN, OFF);
		digitalWrite(HEATER_PIN, OFF);
	}
}

