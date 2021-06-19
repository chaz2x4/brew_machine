#if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
#endif

#include "GCP.h"
#include "PID.h"
#include "OLED.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>