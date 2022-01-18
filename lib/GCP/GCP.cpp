#include "GCP.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(THERMOPROBE_PIN))
, emergencyShutoffTemp(165.0)
, maxBrewTemp(115.0)
, minBrewTemp(85.0)
, maxSteamTemp(160.0)
, minSteamTemp(140.0)
, maxOffset(15)
, minOffset(-15)
, websiteQueueSize(69)
, windowSize(1500)
, logInterval(500)
, powerFrequency(60)
, tempOffset(-8)
, targetTemp(92)
, targetSteamTemp(150)
, lastTime(-1)
, Kp(127.5)
, Ki(15.55)
, Kd(392.06)
, outputQueue(Queue(websiteQueueSize))
, brewTempManager(PID(&currentTemp, &brew_output, &targetTemp, Kp, Ki, Kd, P_ON_M, DIRECT))
, steamTempManager(PID(&currentTemp, &steam_output, &targetSteamTemp, Kp, Ki, Kd, P_ON_M, DIRECT))
{}

GCP::~GCP(){
	digitalWrite(HEATER_PIN, LOW);
	digitalWrite(STEAM_PIN, LOW);
}

void GCP::start() {
	loadParameters();

	this->tempProbe.begin(MAX31865_3WIRE);
	this->getCurrentTemp();

	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTempManager.SetMode(AUTOMATIC);
	steamTempManager.SetMode(AUTOMATIC);
	brewTempManager.SetOutputLimits(0, windowSize);
	steamTempManager.SetOutputLimits(0, windowSize);
	brewTempManager.SetSampleTime(logInterval);
	steamTempManager.SetSampleTime(logInterval);
}

void GCP::setTargetTemp(String currentMode, double temp) {
	double minTemp;
	double maxTemp;
	int tempAddress;
	if(currentMode == "offset") {
		minTemp = minOffset;
		maxTemp = maxOffset;
		tempAddress = OFFSET_ADDRESS;
	}
	else if(currentMode == "steam") {
		minTemp = minSteamTemp;
		maxTemp = maxSteamTemp;
		tempAddress = STEAM_TEMP_ADDRESS;
	}
	else {
		minTemp = minBrewTemp;
		maxTemp = maxBrewTemp;
		tempAddress = BREW_TEMP_ADDRESS;
	}

	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;

	if(currentMode == "offset") this->tempOffset = temp;
	else if(currentMode == "steam") this->targetSteamTemp = temp;
	else this->targetTemp = temp;

	EEPROM.put(tempAddress, temp);
	EEPROM.commit();
}

void GCP::incrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "offset") temp = tempOffset;
	else if(currentMode == "steam") { 
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp += i;
	this->setTargetTemp(currentMode, temp);
}

void GCP::decrementTemp(String currentMode) {
	double temp;
	double i = 0.5;
	if(currentMode == "offset") temp = tempOffset;
	else if(currentMode == "steam") {
		temp = targetSteamTemp;
		i = 1;
	}
	else temp = targetTemp;
	temp -= i;
	this->setTargetTemp(currentMode, temp);
}

double GCP::getTargetTemp(String currentMode) {
	if(currentMode == "steam") return this->targetSteamTemp;
	else if(currentMode == "offset") return this->tempOffset;
	else return this->targetTemp;
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

double GCP::getCurrentTemp() {
	double temp = this->getActualTemp();
	temp += tempOffset;
	this->currentTemp = temp;
	return temp;
}

String GCP::getOutput(){
	String outputString = "[";

	for(unsigned i = 0; i < outputQueue.size(); i++){
		if(i > 0 ) outputString += ",";
		outputString += outputQueue.at(i);
	}

	outputString += "]";
	return outputString;
}

String GCP::getTunings(){
	String output;
    output += "{ \"kp\": ";
    output += Kp;
    output += ", \"ki\": ";
    output += Ki;
    output +=  ", \"kd\": ";
    output += Kd;
    output += " }";
    return output;
}

void GCP::startTimer(ulong realTime){
	this->timerStartTime = realTime;
}

double GCP::getCurrentTimer(ulong now){
	if(brewSwitchOn) return (now - timerStartTime) / 1000;
	else return 0;
}

void GCP::setTunings(double kp, double ki, double kd){
	this->Kp = kp;
	this->Ki = ki;
	this->Kd = kd;

	brewTempManager.SetTunings(kp, ki, kd, P_ON_M);
	brewTempManager.SetMode(AUTOMATIC);

	steamTempManager.SetTunings(kp, ki, kd, P_ON_M);
	steamTempManager.SetMode(AUTOMATIC);
	
	EEPROM.put(TUNING_ADDRESS, kp);
	EEPROM.put(TUNING_ADDRESS + 8, ki);
	EEPROM.put(TUNING_ADDRESS + 16, kd);
	EEPROM.commit();
}

void GCP::parseQueue(ulong time){
	String outputs;
    outputs += "{ \"time\": ";
    outputs += time;
    outputs += ", \"temperature\": ";
    outputs += this->currentTemp;
    outputs += ", \"outputs\": { \"brew\": ";
    outputs += this->lastBrewOutput;
    outputs += ", \"steam\": ";
    outputs += this->lastSteamOutput;
	outputs += "}, \"targets\": { \"brew\": ";
    outputs += this->targetTemp;
    outputs += ", \"steam\": ";
    outputs += this->targetSteamTemp;
	outputs += ", \"offset\": ";
    outputs += this->tempOffset;
    outputs += " }}";
	outputQueue.push(outputs);
}

void GCP::loadParameters(){
	double brewTemp, steamTemp, offset;
	EEPROM.get(BREW_TEMP_ADDRESS, brewTemp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steamTemp);
	EEPROM.get(OFFSET_ADDRESS, offset);

	if(!isnan(brewTemp) && brewTemp >= minBrewTemp && brewTemp <= maxBrewTemp) targetTemp = brewTemp;
	if(!isnan(steamTemp) && steamTemp >= minSteamTemp && steamTemp <= maxSteamTemp) targetSteamTemp = steamTemp;
	if(!isnan(offset) && offset >= minOffset && offset <= maxOffset) tempOffset = offset;

	double tunings[3];
	bool tuningsValid = true;
	for(int i=0; i<3; i++) {
		EEPROM.get(TUNING_ADDRESS + i*8, tunings[i]);
		if(tunings[i] < 0 || isnan(tunings[i])) {
			tuningsValid = false;
			break;
		}
	}
	if(tuningsValid) setTunings(tunings[0], tunings[1], tunings[2]);
}

double GCP::sensedCurrent() {
	return 1;
}

int GCP::regulateOutput(double output) {
	int roundedOutput = int(output);
	int powerPeriod = int(1000/powerFrequency);
	int remainder = roundedOutput % powerPeriod;
	if(remainder == 0) return roundedOutput;
	return roundedOutput + powerPeriod - remainder;
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

	double currentCurrent = sensedCurrent();
	double currentTemp;
	if(!brewSwitchOn && currentCurrent > 0) {
		brewSwitchOn = true;
		this->startTimer(realTime);
	} else if(currentCurrent == 0) brewSwitchOn = false;

	ulong now = millis();
	if(now - logStartTime > logInterval) {
		currentTemp = this->getCurrentTemp();
   		brewTempManager.Compute();
		steamTempManager.Compute();

		if(lastTime < realTime) parseQueue(realTime);
		lastTime = realTime;
		logStartTime += logInterval;
	}
	
	if(now - windowStartTime > windowSize) {
		windowStartTime += windowSize;
		lastBrewOutput = regulateOutput(brew_output);
		lastSteamOutput = regulateOutput(steam_output);

		if(targetSteamTemp - currentTemp > 10) lastSteamOutput = windowSize;
		if(targetTemp - currentTemp > 10) lastBrewOutput = windowSize;
		if(targetSteamTemp - currentTemp < -1) lastSteamOutput = 0;
		if(targetTemp - currentTemp < -1) lastBrewOutput = 0;
	}

	if(lastBrewOutput > now - windowStartTime) digitalWrite(HEATER_PIN, HIGH);
	else digitalWrite(HEATER_PIN, LOW);

	if(lastSteamOutput > now - windowStartTime) digitalWrite(STEAM_PIN, HIGH);
	else digitalWrite(STEAM_PIN, LOW);
	
	double actualTemp = this->getActualTemp();
	if(actualTemp >= emergencyShutoffTemp) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
	}
}