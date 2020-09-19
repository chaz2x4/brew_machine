#include "gcp.h"

GCP::GCP() {
	GCP(DEFAULT_TARGET_TEMP);
}

GCP::GCP(float targetTemp) {
	init(targetTemp);
}

void GCP::init(float targetTemp){
	this->tempProbe.begin(MAX31865_3WIRE);
	this->setTargetTemp(targetTemp);
	float actualTemp = this->getActualTemp();
	this->temperatureManager.initialize(targetTemp, actualTemp);
}

GCP::~GCP() {
	float targetTemp = this->getTargetTemp(); //get current target temp
	if (targetTemp > MAX_BREW_TEMP) targetTemp = DEFAULT_TARGET_TEMP; //if it's higher than max value, then set it to default value of 95.0
};

void GCP::setTargetTemp(float temp) {
	setTargetTemp(temp, MIN_BREW_TEMP, MAX_BREW_TEMP); //Set max default brew temperature to between 90 - 96 degrees
}

void GCP::setTargetTemp(float temp, float minTemp, float maxTemp) {
	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;
	this->targetTemp = temp;
}

void GCP::incrementTemp() {
	float temp = this->getTargetTemp();
	this->setTargetTemp(temp + 0.5);
}

void GCP::decrementTemp() {
	float temp = this->getTargetTemp();
	this->setTargetTemp(temp - 0.5);
}

float GCP::getTargetTemp() {
	return this->targetTemp;
}

float GCP::getPX() {
	return this->pressure;
}

float GCP::getActualTemp() {
	float temp =  tempProbe.temperature(100, RREF);
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
	float targetTemp = this->getTargetTemp();
	float actualTemp = this->getActualTemp();
	float pressure = this->getPX();

	temperatureManager.compute(targetTemp, actualTemp);
	this->heating_switch = temperatureManager.isHeaterRunning();

	Serial.printf("Set Temp: %0.1f; Actual Temp: %0.1f; Pressure: %0.1f bar \n", targetTemp, actualTemp, pressure);

	if(actualTemp >= EMERGENCY_SHUTOFF_TEMP) {
		heating_switch = OFF;
		steam_switch = OFF;
	}

	digitalWrite(HEATER_PIN, heating_switch);
	digitalWrite(STEAM_PIN, steam_switch);
}
