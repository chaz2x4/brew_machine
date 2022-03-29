#if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>

#include "passwords.h"
#include "websites.h"
#include "gcp.h"
#include "oled.h"