#include "gcp.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(THERMOPROBE_PIN))
, currentSensor(ACS712(ACS_VERSION, CURRENT_PIN))
, kEmergencyShutoffTemp(165.0)
, kMaxBrewTemp(115.0)
, kMinBrewTemp(85.0)
, kMaxSteamTemp(150.0)
, kMinSteamTemp(130.0)
, kMaxOffset(15)
, kMinOffset(-15)
, kWebsiteQueueSize(69)
, kWindowSize(1500)
, kLogInterval(500)
, kPowerFrequency(60)
, kTranducerLimit(12)
, temp_offset(-8)
, target_temp(92)
, target_steam_temp(140)
, Kp(127.5)
, Ki(15.55)
, Kd(392.06)
, outputQueue(Queue(kWebsiteQueueSize))
, brewTempManager(PID(&current_temp, &brew_output, &target_temp, Kp, Ki, Kd, P_ON_M, DIRECT))
, steamTempManager(PID(&current_temp, &steam_output, &target_steam_temp, Kp, Ki, Kd, P_ON_M, DIRECT))
{}

GCP::~GCP(){
	digitalWrite(HEATER_PIN, LOW);
	digitalWrite(STEAM_PIN, LOW);
}

void GCP::start() {
	loadParameters();

	this->tempProbe.begin(MAX31865_3WIRE);
	this->currentSensor.calibrate();
	this->getCurrentTemp();

	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	brewTempManager.SetMode(AUTOMATIC);
	steamTempManager.SetMode(AUTOMATIC);
	brewTempManager.SetOutputLimits(0, kWindowSize);
	steamTempManager.SetOutputLimits(0, kWindowSize);
	brewTempManager.SetSampleTime(kLogInterval);
	steamTempManager.SetSampleTime(kLogInterval);
}

void GCP::setTargetTemp(String current_mode, double temp) {
	double minTemp;
	double maxTemp;
	int tempAddress;
	if(current_mode == "offset") {
		minTemp = kMinOffset;
		maxTemp = kMaxOffset;
		tempAddress = OFFSET_ADDRESS;
	}
	else if(current_mode == "steam") {
		minTemp = kMinSteamTemp;
		maxTemp = kMaxSteamTemp;
		tempAddress = STEAM_TEMP_ADDRESS;
	}
	else {
		minTemp = kMinBrewTemp;
		maxTemp = kMaxBrewTemp;
		tempAddress = BREW_TEMP_ADDRESS;
	}

	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;

	if(current_mode == "offset") this->temp_offset = temp;
	else if(current_mode == "steam") this->target_steam_temp = temp;
	else this->target_temp = temp;

	EEPROM.put(tempAddress, temp);
	EEPROM.commit();
}

void GCP::incrementTemp(String current_mode) {
	double temp;
	double i = 0.5;
	if(current_mode == "offset") temp = temp_offset;
	else if(current_mode == "steam") { 
		temp = target_steam_temp;
		i = 1;
	}
	else temp = target_temp;
	temp += i;
	this->setTargetTemp(current_mode, temp);
}

void GCP::decrementTemp(String current_mode) {
	double temp;
	double i = 0.5;
	if(current_mode == "offset") temp = temp_offset;
	else if(current_mode == "steam") {
		temp = target_steam_temp;
		i = 1;
	}
	else temp = target_temp;
	temp -= i;
	this->setTargetTemp(current_mode, temp);
}

double GCP::getTargetTemp(String current_mode) {
	if(current_mode == "steam") return this->target_steam_temp;
	else if(current_mode == "offset") return this->temp_offset;
	else return this->target_temp;
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

double GCP::getPressure() {
	double pxRead = analogRead(TRANSDUCER_PIN);
	double px = (pxRead / 4096) * kTranducerLimit;
	return px;
}

double GCP::getCurrentTemp() {
	double temp = this->getActualTemp();
	temp += temp_offset;
	this->current_temp = temp;
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
    outputs += this->current_temp;
    outputs += ", \"outputs\": { \"brew\": ";
    outputs += this->last_brew_output;
    outputs += ", \"steam\": ";
    outputs += this->last_steam_output;
	outputs += "}, \"targets\": { \"brew\": ";
    outputs += this->target_temp;
    outputs += ", \"steam\": ";
    outputs += this->target_steam_temp;
	outputs += ", \"offset\": ";
    outputs += this->temp_offset;
    outputs += " }}";
	outputQueue.push(outputs);
}

void GCP::loadParameters(){
	double brew_temp, steam_temp, offset;
	EEPROM.get(BREW_TEMP_ADDRESS, brew_temp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steam_temp);
	EEPROM.get(OFFSET_ADDRESS, offset);

	if(!isnan(brew_temp) && brew_temp >= kMinBrewTemp && brew_temp <= kMaxBrewTemp) target_temp = brew_temp;
	if(!isnan(steam_temp) && steam_temp >= kMinSteamTemp && steam_temp <= kMaxSteamTemp) target_steam_temp = steam_temp;
	if(!isnan(offset) && offset >= kMinOffset && offset <= kMaxOffset) temp_offset = offset;

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

bool GCP::isBrewing() {
	double currentAC = currentSensor.getCurrentAC();
	return currentAC > 1;
}

ulong GCP::getBrewStartTime() {
	return brew_start_time;
}

ulong GCP::getBrewStopTime() {
		return brew_stop_time;
}

int GCP::regulateOutput(double output) {
	int roundedOutput = int(output);
	int powerPeriod = int(1000/kPowerFrequency);
	int remainder = roundedOutput % powerPeriod;
	if(remainder == 0) return roundedOutput;
	return roundedOutput + powerPeriod - remainder;
}

void GCP::refresh(ulong real_time) {
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

	if(isBrewing() && brew_start_time == 0) brew_start_time = real_time;
	else if(!isBrewing() && brew_start_time > 0) {
		brew_start_time = 0;
		brew_stop_time = real_time;
	}

	double current_temp;
	ulong now = millis();
	if(now - log_start_time > kLogInterval) {
		current_temp = this->getCurrentTemp();
		brewTempManager.Compute();
		steamTempManager.Compute();
		parseQueue(real_time);
		log_start_time += kLogInterval;
	}
	
	if(now - window_start_time > kWindowSize) {
		window_start_time += kWindowSize;
		last_brew_output = regulateOutput(brew_output);
		last_steam_output = regulateOutput(steam_output);

		if(target_steam_temp - current_temp > 5) {
			steamTempManager.SetMode(MANUAL);
			last_steam_output = kWindowSize;
		}
		else if(target_steam_temp - current_temp < -1) { 
			steamTempManager.SetMode(MANUAL);
			last_steam_output = 0;
		}
		else {
			steamTempManager.SetMode(AUTOMATIC);
		}
		
		if(target_temp - current_temp > 5)  {
			brewTempManager.SetMode(MANUAL);
			last_brew_output = kWindowSize;
		}
		else if(target_temp - current_temp < -1) {
			brewTempManager.SetMode(MANUAL);
			last_brew_output = 0;
		} else {
			brewTempManager.SetMode(AUTOMATIC);
		}
	}

	if(last_brew_output > now - window_start_time) digitalWrite(HEATER_PIN, HIGH);
	else digitalWrite(HEATER_PIN, LOW);

	if(last_steam_output > now - window_start_time) digitalWrite(STEAM_PIN, HIGH);
	else digitalWrite(STEAM_PIN, LOW);
	
	double actualTemp = this->getActualTemp();
	if(actualTemp >= kEmergencyShutoffTemp) {
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
	}
}