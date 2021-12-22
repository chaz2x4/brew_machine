#include "GCP.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(A5))
, emergencyShutoffTemp(165.0)
, maxBrewTemp(100.0)
, minBrewTemp(85.0)
, maxSteamTemp(160.0)
, minSteamTemp(40.0)
, maxOffset(15)
, minOffset(-15)
, websiteQueueSize(60)
, cycleTime(2000)
, tempOffset(-8)
, targetTemp(92)
, targetSteamTemp(150)
, outputQueue(Queue(websiteQueueSize))
, brewTempManager(PID(&currentTemp, &brew_output, &targetTemp, 68.4, 44.34, 1.5, P_ON_M, DIRECT))
, steamTempManager(PID(&currentTemp, &steam_output, &targetSteamTemp, 68.4, 44.34, 1, P_ON_M, DIRECT))    
{}

GCP::~GCP(){
}

void GCP::start() {
	this->tempProbe.begin(MAX31865_3WIRE);
	this->currentTemp = this->getCurrentTemp();

	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTempManager.SetMode(AUTOMATIC);
	steamTempManager.SetMode(AUTOMATIC);
	brewTempManager.SetOutputLimits(0, cycleTime);
	steamTempManager.SetOutputLimits(0, cycleTime);
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

void GCP::incrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "steam") { 
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp += i;
	if(currentMode == "steam") this->setTargetSteamTemp(temp);
	else this->setTargetTemp(temp);
}

void GCP::decrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "steam") {
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp -= i;
	if(currentMode == "steam") this->setTargetSteamTemp(temp);
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
	return temp;
}

double GCP::getCurrentTemp() {
	double temp = this->getActualTemp();
	temp += tempOffset;
	this->currentTemp = temp;
	return temp;
}

double GCP::getTempOffset(){
	return this->tempOffset;
}

void GCP::setTempOffset(double offset){
	if(offset > maxOffset) this->tempOffset = maxOffset;
	else if(offset < minOffset) this->tempOffset = minOffset;
	else this->tempOffset = offset;
}

String GCP::getOutput(){
	return this->outputString;
}

String GCP::getTunings(String currentMode){
	String output;
	PID *tempManager;
	if(currentMode == "steam") tempManager = &steamTempManager;
	else tempManager = &brewTempManager;
    output += "{ \"kp\": ";
    output += tempManager->GetKp();
    output += ", \"ki\": ";
    output += tempManager->GetKi();
    output +=  ", \"kd\": ";
    output += tempManager->GetKd();
    output += " }";
    return output;
}

void GCP::setTunings(String currentMode, double kp, double ki, double kd){
	if(currentMode == "steam") steamTempManager.SetTunings(kp, ki, kd, P_ON_M);
	else brewTempManager.SetTunings(kp, ki, kd, P_ON_M);
}

void GCP::parseQueue(ulong time){
	String outputs;
    outputs += "{ \"time\": ";
    outputs += time;
    outputs += ", \"temperature\": ";
    outputs += this->getCurrentTemp();
    outputs += ", \"outputs\": { \"brew\": ";
    outputs += this->brew_output;
    outputs += ", \"steam\": ";
    outputs += this->steam_output;
    outputs += " }}";
	outputQueue.push(outputs);

	outputString = "{ \"targets\": { \"brew\": ";
	outputString += this->targetTemp;
	outputString += ", \"steam\": ";
	outputString += this->targetSteamTemp;
	outputString += ", \"offset\": ";
	outputString += this->tempOffset;
	outputString += " }, \"outputs\":[";

	for(unsigned i = 0; i < outputQueue.size(); i++){
		if(i > 0 ) outputString += ",";
		outputString += outputQueue.at(i);
	}

	outputString += "]}";
}

void GCP::refresh(ulong realTime) {
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

	ulong runTime = millis();
	if(runTime - cycleStartTime > cycleTime) {
		parseQueue(realTime);
		cycleStartTime += cycleTime;
		brewTempManager.Compute();
		steamTempManager.Compute();
	}
	
	if(brew_output > runTime - cycleStartTime) digitalWrite(HEATER_PIN, HIGH);
	else digitalWrite(HEATER_PIN, LOW);

	if(steam_output > runTime - cycleStartTime) digitalWrite(STEAM_PIN, HIGH);
	else digitalWrite(STEAM_PIN, LOW);
	
	double actualTemp = this->getActualTemp();
	if((actualTemp + tempOffset) >= emergencyShutoffTemp) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
	}
}