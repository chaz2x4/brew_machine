#include "gcp.h"

GCP::GCP()
: tempProbe(Adafruit_MAX31865(THERMOPROBE_PIN))
, kEmergencyShutoffTemp(165.0)
, kMaxBrewTemp(105.0)
, kMinBrewTemp(85.0)
, kMaxSteamTemp(150.0)
, kMinSteamTemp(130.0)
, kMaxOffset(11)
, kMinOffset(-11)
, kWindowSize(1000)
, kLogInterval(500)
, kOutputStep(50)
, kTuneTime(500)
, kSamples(500)
, kSettleTime(10)
, temp_offset(-8)
, target_brew_temp(92)
, target_steam_temp(140)
, brew_output(0)
, steam_output(0)
, tuner_output(0)
, kp(40)
, ki(6.67)
, kd(52.27)
, tuner(&current_temp, &tuner_output, tuner.CohenCoon_PID, tuner.directIP, tuner.printALL)
, brewTempManager(QuickPID(&current_temp, &brew_output, &target_brew_temp))
, steamTempManager(QuickPID(&current_temp, &steam_output, &target_steam_temp))
, outputQueue(Queue(60000 / kLogInterval, C))
{}

GCP::~GCP(){
	digitalWrite(HEATER_PIN, LOW);
	digitalWrite(STEAM_PIN, LOW);
}

void GCP::start() {
	loadParameters();

	if(!tempProbe.begin(MAX31865_3WIRE)) {
		Serial.println("Could not initialize thermocouple.");
		while (1) delay(10);
	}

	pinMode(THERMOPROBE_READY_PIN, INPUT);
	pinMode(HEATER_PIN, OUTPUT);
	pinMode(STEAM_PIN, OUTPUT);

	tuner.Configure(kMaxSteamTemp, kWindowSize, 0, kOutputStep, kTuneTime, kSettleTime, kSamples);
	tuner.SetEmergencyStop(kEmergencyShutoffTemp);
}

void GCP::setTargetTemp(TempMode current_mode, float temp) {
	float minTemp;
	float maxTemp;
	int tempAddress;
	switch(current_mode) {
		case OFFSET:
			minTemp = kMinOffset;
			maxTemp = kMaxOffset;
			tempAddress = OFFSET_ADDRESS;
			break;
		case STEAM:
			minTemp = kMinSteamTemp;
			maxTemp = kMaxSteamTemp;
			tempAddress = STEAM_TEMP_ADDRESS;
			break;
		default:
			minTemp = kMinBrewTemp;
			maxTemp = kMaxBrewTemp;
			tempAddress = BREW_TEMP_ADDRESS;
	}

	if (temp < minTemp) temp = minTemp;
	else if (temp > maxTemp) temp = maxTemp;

	switch(current_mode) {
		case OFFSET:
			this->temp_offset = temp;
			break;
		case STEAM:
			this->target_steam_temp = temp;
			break;
		default:
			this->target_brew_temp = temp;
	}

	EEPROM.put(tempAddress, temp);
	EEPROM.commit();
}

void GCP::incrementTemp(String current_mode) {
	incrementTemp(modeToEnum(current_mode));
}

void GCP::incrementTemp(TempMode current_mode) {
	float temp;
	float i = 0.5;

	if(outputQueue.scale == F) i = (5.0/9.0);
	else if (current_mode == STEAM) i = 1;

	switch(current_mode) {
		case OFFSET: temp = temp_offset;
			break;
		case STEAM: temp = target_steam_temp;
			break;
		default: temp = target_brew_temp;
	}

	temp += i;
	this->setTargetTemp(current_mode, temp);
}

void GCP::decrementTemp(String current_mode) {
	decrementTemp(modeToEnum(current_mode));
}

void GCP::decrementTemp(TempMode current_mode) {
	float temp;
	float i = 0.5;

	if(outputQueue.scale == F) i = (5.0/9.0);
	else if (current_mode == STEAM) i = 1;

	switch(current_mode) {
		case OFFSET: temp = temp_offset;
			break;
		case STEAM: temp = target_steam_temp;
			break;
		default: temp = target_brew_temp;
	}

	temp -= i;
	this->setTargetTemp(current_mode, temp);
}

float GCP::getTargetTemp(TempMode current_mode) {
	switch(current_mode) {
		case OFFSET: 
			return this->temp_offset;
		case STEAM:
			return this->target_steam_temp;
			default: return this->target_brew_temp;
	}
}

float GCP::getActualTemp() {
	float temp = tempProbe.temperature(100, RREF);
 	uint8_t probe_fault = tempProbe.readFault();
	if (probe_fault) {
		Serial.print("Fault 0x"); Serial.println(probe_fault, HEX);
		if (probe_fault & MAX31865_FAULT_HIGHTHRESH) Serial.println("RTD High Threshold");
		if (probe_fault & MAX31865_FAULT_LOWTHRESH) Serial.println("RTD Low Threshold"); 
		if (probe_fault & MAX31865_FAULT_REFINLOW) Serial.println("REFIN- > 0.85 x Bias");
		if (probe_fault & MAX31865_FAULT_REFINHIGH) Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
		if (probe_fault & MAX31865_FAULT_RTDINLOW) Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
		if (probe_fault & MAX31865_FAULT_OVUV) Serial.println("Under/Over voltage");
		tempProbe.clearFault();
	}
	return temp;
}

float GCP::getCurrentTemp() {
	float temp = this->getActualTemp();
	temp += temp_offset;
	this->current_temp = temp;
	return temp;
}

String GCP::getOutput(){
	return outputQueue.toJson();
}

const char* GCP::getScale(){
	return outputQueue.scale == F ? "F" : "C";
}

String GCP::getTunings(){
	StaticJsonDocument<64> output;
	output["kp"] = kp;
	output["ki"] = ki;
	output["kd"] = kd;
	String outputString;
	serializeJson(output, outputString);
	return outputString;
}

void GCP::PWM(TempMode mode, QuickPID* tempManager, int pin, float output, float target_temp) {
	float optimumOutput = tuner.softPwm(pin, current_temp, tuner_output, target_temp, kWindowSize, debounce);
	switch(tuner.Run()) {
		case tuner.sample: //Runs once per sample during test phase
			this->getCurrentTemp();
			tuner.plotter(current_temp, tuner_output, target_temp, 0.5f, 3);
			break;

		case tuner.tunings: //Set tunings once test is complete
			tuner.GetAutoTunings(&kp, &ki, &kd);
			tempManager->SetOutputLimits(0, kWindowSize * 0.1);
			tempManager->SetSampleTimeUs((kWindowSize - 1) * 1000);
			debounce = false;
			tuner_output = kOutputStep;
			setTunings(kp, ki, kd);
			break;

		case tuner.runPid: //Runs once per sample after tunings set
			this->getCurrentTemp();
			brewTempManager.Compute();
			steamTempManager.Compute();
			tuner_output = mode == BREW ? brew_output : steam_output;
			tuner.plotter(current_temp, optimumOutput, target_brew_temp, 0.5f, 3);
			break;
	}
}

void GCP::setTunings(float kp, float ki, float kd){
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;

	brewTempManager.SetMode(brewTempManager.Control::automatic);
	steamTempManager.SetMode(steamTempManager.Control::automatic);

	brewTempManager.SetProportionalMode(brewTempManager.pMode::pOnMeas);
	steamTempManager.SetProportionalMode(steamTempManager.pMode::pOnMeas);

	brewTempManager.SetAntiWindupMode(brewTempManager.iAwMode::iAwClamp);
	steamTempManager.SetAntiWindupMode(steamTempManager.iAwMode::iAwClamp);

	brewTempManager.SetTunings(kp, ki, kd);
	steamTempManager.SetTunings(kp, ki, kd);

	EEPROM.put(TUNING_ADDRESS, kp);
	EEPROM.put(TUNING_ADDRESS + 8, ki);
	EEPROM.put(TUNING_ADDRESS + 16, kd);
	EEPROM.commit();
}

void GCP::changeScale(String scale_string) {
	TempScale scale = scale_string == "F" ? F : C;
	outputQueue.scale = scale;
	EEPROM.put(SCALE_ADDRESS, scale);
	EEPROM.commit();
}

TempMode GCP::modeToEnum(String mode) {
	if(mode == "offset") return OFFSET;
	else if(mode == "steam") return STEAM;
	else return BREW;
}

void GCP::loadParameters(){
	float brew_temp, steam_temp, offset;
	TempScale scale;
	EEPROM.get(BREW_TEMP_ADDRESS, brew_temp);
	EEPROM.get(STEAM_TEMP_ADDRESS, steam_temp);
	EEPROM.get(OFFSET_ADDRESS, offset);
	EEPROM.get(SCALE_ADDRESS, scale);

	if(!isnan(brew_temp) && brew_temp >= kMinBrewTemp && brew_temp <= kMaxBrewTemp) target_brew_temp = brew_temp;
	if(!isnan(steam_temp) && steam_temp >= kMinSteamTemp && steam_temp <= kMaxSteamTemp) target_steam_temp = steam_temp;
	if(!isnan(offset) && offset >= kMinOffset && offset <= kMaxOffset) temp_offset = offset;
	if(scale) outputQueue.scale = scale;

	float tunings[3];
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

	this->PWM(BREW, &brewTempManager, HEATER_PIN, brew_output, target_brew_temp);
	this->PWM(STEAM, &steamTempManager, STEAM_PIN, steam_output, target_steam_temp);

	//Log information for website display
	ulong now = millis();
	ulong log_time_elapsed = now - log_start_time;
	if(log_time_elapsed > kLogInterval) {
		outputQueue.push(
			real_time, 
			this->getCurrentTemp(), 
			this->brew_output, 
			this->steam_output,
			this->target_brew_temp,
			this->target_steam_temp,
			this->temp_offset
		);
		log_start_time += kLogInterval;
	}
}