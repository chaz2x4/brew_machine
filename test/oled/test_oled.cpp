#include "header.h"
#include "unity.h"

OLED display;

void setup() {
    delay(2000);
    UNITY_BEGIN();
    display.start();
}

void loop () {
    UNITY_END();
}