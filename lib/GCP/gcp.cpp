#include "gcp.h"

GCP::GCP() {
	GCP(DEFAULT_TARGET_TEMP);
}

GCP::GCP(float targetTemp) {
	init(targetTemp);
}

void GCP::init(float targetTemp){
	float actualTemp = this->getActualTemp();
	this->setTargetTemp(targetTemp);
	this->power_light = ON;
	this->temperatureManager.compute(targetTemp, actualTemp);
}

GCP::~GCP() {
	float targetTemp = this->getTargetTemp(); //get current target temp
	if (targetTemp > MAX_BREW_TEMP) targetTemp = DEFAULT_TARGET_TEMP; //if it's higher than max value, then set it to default value of 95.0
	EEPROM.put(0, targetTemp); //save target temp to hardware for next startup
};

void GCP::setTargetTemp(float temp) {
	setTargetTemp(temp, MIN_BREW_TEMP, MAX_BREW_TEMP); //Set max default brew temperature to between 90 - 96 degrees
}

void GCP::setTargetTemp(float temp, float minTemp, float maxTemp) {
	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;
	this->targetTemp = temp;
	EEPROM.put(0, temp);
}

void GCP::incrementTemp() {
	int temp = this->getTargetTemp();
	this->setTargetTemp(temp + 0.5);
}

void GCP::decrementTemp() {
	int temp = this->getTargetTemp();
	this->setTargetTemp(temp - 0.5);
}


float GCP::getTargetTemp() {
	//if a temperature value was saved in the EEPROM, then retrieve it
	return this->targetTemp;
}

float GCP::getPX() {
	return this->pressure;
}

float GCP::getActualTemp() {
	return this->actualTemp;
}

bool GCP::isSteamReady() {
	return this->steam_light;
}

bool GCP::isBrewReady() {
	return this->brew_light;
}

bool GCP::isPowerOn() {
	return this->power_light;
}

void GCP::update() {
	float targetTemp = this->getTargetTemp();
	float actualTemp = this->getActualTemp();
	this->temperatureManager.compute(targetTemp, actualTemp);

	//Turn on lights if within 1 degree C of target temp
	float error = targetTemp - actualTemp;
	if (error >= -1.0 || error <= 1.0) {
		bool steam_switch = this->steam_switch;
		if (steam_switch) {
			this->steam_light = ON; 
			this->brew_light = OFF;
		}
		else { 
			this->brew_light = ON; 
			this->steam_light = OFF;
		}
	}
	else {
		this->steam_light = OFF;
		this->brew_light = OFF;
	}
}
