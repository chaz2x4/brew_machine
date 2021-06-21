#include "header.h"

WebServer server(80);
OLED brew_machine;
char *hostname = "gaggia";

void setup() {
	while (!Serial) ; // wait for serial port to connect. Needed for native USB port only
	//Primary Mission: start coffee machine
	Serial.begin(115200);
	Serial.setDebugOutput(true);

	WiFi.mode(WIFI_STA);
	WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
	  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.print("IP Address: ");
	Serial.print(WiFi.localIP());
	Serial.println("");

	if (!MDNS.begin(hostname)) { //http://gaggia.local
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("Running on http://gaggia.local");

	server.on("/", HTTP_GET, []() {
		server.send(200, "text/html", indexHtml);
	});

	server.on("/get_temps", HTTP_GET, [](){
		server.send(200, "text/json", brew_machine.getOutput());
	});

	server.on("/get_tunings", HTTP_GET, [](){
		server.send(200, "text/json", brew_machine.getTunings());
	});

	server.on("/set_tunings", HTTP_POST, [](){
		server.send(200, "text/plain", "");
	});

	server.on("/increment_target", HTTP_POST, [](){
		String message = "Arguments: ";
		message += server.args();
		message += "\n";
		for (uint8_t i = 0; i < server.args(); i++) {
			message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
		}

		server.send(200, "text/plain", message);
	});

	server.on("/decrement_target", HTTP_POST, [](){
		server.send(200, "text/plain", "");
	});

	server.on("/set_offset", HTTP_POST, [](){
		server.send(200, "text/plain", "");
	});

	server.onNotFound([]() {
		server.send(404, "text/plain", "Not Found");
	});

	ArduinoOTA.setHostname(hostname);
	ArduinoOTA.onStart([](){
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
		else type = "filesystem";
		Serial.println("Updating " + type);
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
	server.begin();
	brew_machine.start();
}

// the loop function runs over and over again until power down or reset
void loop() {
	ArduinoOTA.handle();
	server.handleClient();
	brew_machine.refresh();
	brew_machine.eventListener();
	yield();
}