#include "header.h"

AsyncWebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
OLED screen;
GCP brew_machine;
const char* hostname = "gaggia";
bool is_wifi_connected = false;

void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	is_wifi_connected = true;
	ArduinoOTA.setHostname(hostname);
	ArduinoOTA.onStart([](){
		digitalWrite(HEATER_PIN, LOW);
		digitalWrite(STEAM_PIN, LOW);
		Serial.println("Updating...");
	}).onEnd([](){
		Serial.println("\nComplete");
	}).onProgress([](uint progress, uint total){
		Serial.printf("Progress %u%%\r", (progress / (total / 1000)));
	}).onError([](ota_error_t error){
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();

	if (!MDNS.begin(hostname)) { //http://gaggia.local
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("Running on http://gaggia.local");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send_P(200, "text/html", indexHtml);
	});

	server.on("/get_temps", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/json", brew_machine.getOutput());
	});

	server.on("/get_tunings", HTTP_GET, [](AsyncWebServerRequest *request){
		if(request->hasArg("mode")) { 
			request->send(200, "text/json", brew_machine.getTunings(request->arg("mode")));
		}
		else request->send(400);
	});

	AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/set_tunings", [](AsyncWebServerRequest *request, JsonVariant &json) {
		JsonObject&& tunings = json.as<JsonObject>();
		if(tunings["kp"].isNull() || tunings["ki"].isNull() || tunings["kd"].isNull() || !request->hasArg("mode")) {
			request->send(400);
		}
		else {
			float kp = tunings["kp"].as<float>();
			float ki = tunings["ki"].as<float>();
			float kd = tunings["kd"].as<float>();
			brew_machine.setTunings(request->arg("mode"), kp, ki, kd);
			request->send(200);
		}
	});
	server.addHandler(handler);

	server.on("/increment_target", HTTP_POST, [](AsyncWebServerRequest *request){
		if(request->hasArg("mode")) { 
			brew_machine.incrementTemp(request->arg("mode"));
			request->send(200);
		}
		else request->send(400);
	});

	server.on("/decrement_target", HTTP_POST, [](AsyncWebServerRequest *request){
		if(request->hasArg("mode")) { 
			brew_machine.decrementTemp(request->arg("mode"));
			request->send(200);
		}
		else request->send(400);
	});

	server.on("/change_scale", HTTP_POST, [](AsyncWebServerRequest *request){
		if(request->hasArg("scale")) {
			brew_machine.changeScale(request->arg("scale"));
			request->send(200);
		}
		else request->send(400);
	});

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(404);
	});

	server.begin();
	timeClient.begin();
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
}

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);

	EEPROM.begin(512);
	brew_machine.start();
	screen.start(&brew_machine);

	WiFi.mode(WIFI_STA);
	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
	WiFi.onEvent(WiFiConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

// the loop function runs over and over again until power down or reset
void loop() {
	ulong current_time;
	if(is_wifi_connected) {
		ArduinoOTA.handle();
		timeClient.update();
		current_time = timeClient.getEpochTime();
	}
	else current_time = millis();

	if(current_time) {
		brew_machine.refresh(current_time);
		screen.refresh(current_time);
	}
}