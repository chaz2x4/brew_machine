#include "oled.h"

void OLED::start(){
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.dim(true);
    display.display();
    delay(1000);

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
    timeLastButton = millis();

    gcp.init();
}

void OLED::eventListener(){
    buttonState[0] = digitalRead(BUTTON_A);
    buttonState[1] = digitalRead(BUTTON_B);
    buttonState[2] = digitalRead(BUTTON_C);

    if(timedout()) { //on any button press immediately turn screen on, but dont do anything else
        if((buttonState[0] == LOW && lastButtonState[0] == HIGH) ||
            (buttonState[1] == LOW && lastButtonState[1] == HIGH) ||
            (buttonState[2] == LOW && lastButtonState[2] == HIGH)) {
                timeLastButton = millis();
                lastButtonState[0] = buttonState[0];
                lastButtonState[1] = buttonState[1];
                lastButtonState[2] = buttonState[2];
        }
    }
    else {
        //Increase temperature when editable on button A
        if(buttonState[0] == LOW && lastButtonState[0] == HIGH) {
            downTime = millis();
            timeLastButton = millis();
            if(this->isEditable) gcp.incrementTemp();
            else this->changeMode();
        }
        else if(buttonState[0] == HIGH && lastButtonState[0] == LOW) downTime = -1 ;
        lastButtonState[0] = buttonState[0];

        //Decrease temperature when editable on button B
        if(buttonState[1] == LOW && lastButtonState[1] == HIGH) {
            downTime = millis();
            timeLastButton = millis();
            if(this->isEditable) gcp.decrementTemp();
            else this->changeMode();
        }
        else if(buttonState[1] == HIGH && lastButtonState[1] == LOW) downTime = -1 ;
        lastButtonState[1] = buttonState[1];

        //Change mode if button C is pressed
        if(buttonState[2] == LOW && lastButtonState[2] == HIGH) {
            downTime = millis();
            timeLastButton = millis();
            if(this->isEditable) {
                this->isEditable = false;
                lastTime = millis();
                flash = true;
            }
        }
        else if(buttonState[2] == HIGH && lastButtonState[2] == LOW) downTime = -1; 
        if(buttonState[2] == LOW && (millis() - downTime) >= TRIGGER_TIME) this->isEditable = true;
        lastButtonState[2] = buttonState[2];
        display.clearDisplay();
    }
}

bool OLED::timedout(){
    if((millis() - timeLastButton) >= SCREEN_TIMEOUT) {
        this->isEditable = false;
        display.clearDisplay();
        display.display();
        return true;
    }
    return false;
}

void OLED::refresh(){
    gcp.update();
    this->getOutput();
    if(timedout()) return;
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 0);
    display.clearDisplay();

    ulong wait;
    String currentMode = "Brew";
    double targetTemp = gcp.getTargetTemp();
    double currentTemp = gcp.getActualTemp();
    if(gcp.getCurrentMode() == steam) {
        currentMode = "Steam";
        targetTemp = gcp.getTargetSteamTemp();
    }
    if(this->isEditable) {
        if(flash) display.printf("Set %s\n %#.1f C", currentMode, targetTemp);
        else display.printf("Set %s", currentMode);
        wait = 500;
    }
    else {
        if(flash) display.printf("%sing\n %#0.1f C", currentMode, targetTemp);
        else display.printf("Temp:\n %#.1f C", currentTemp);
        wait = 2000;
    }

    if((millis() - lastTime) >= wait) {
        lastTime = millis();
        flash = !flash;
    }
    display.display();
}

void OLED::changeMode(){
    display.clearDisplay();
    lastTime = millis();
    flash = true;
    if(gcp.getCurrentMode() == brew) gcp.setMode(steam);
    else gcp.setMode(brew);
}

void OLED::setMode(mode mode){
    Serial.printf("Set Mode %i\n", mode);
    gcp.setMode(mode);
}

void OLED::incrementTemp(){
    Serial.printf("Increment Temp %i\n", gcp.getCurrentMode());
    gcp.incrementTemp();
}

void OLED::decrementTemp(){
    Serial.printf("Decrement Temp %i\n", gcp.getCurrentMode());
    gcp.decrementTemp();
}

void OLED::setOffset(double offset) {
    Serial.printf("Set Offset: %f\n", offset);
    gcp.setTempOffset(offset);
}

void OLED::setTunings(double kp, double ki, double kd){
    Serial.printf("Set Tuning: %f %f %f\n", kp, ki, kd);
    gcp.setTunings(kp, ki, kd);
}

String OLED::getOutput(){
    String output;
    output += "{ \"temperature\": ";
    output += gcp.getActualTemp();
    output += ", \"offset\": ";
    output += gcp.getTempOffset();
    output += ", \"brew\": { \"target\": ";
    output += gcp.getTargetTemp(); 
    output += ", \"output\": ";
    output += gcp.getBrewOutput();
    output += " } , \"steam\": { \"target\": ";
    output += gcp.getTargetSteamTemp();
    output +=  ", \"output\": ";
    output += gcp.getSteamOutput();
    output += " }}";
    Serial.println(output);
    return output;
}

String OLED::getTunings(){
    String output;

    double* tunings = gcp.getTunings();
    output += "{ \"kp\": ";
    output += tunings[0];
    output += ", \"ki\": ";
    output += tunings[1];
    output +=  ", \"kd\": ";
    output += tunings[2];
    output += " }";
    Serial.println(output);
    return output;
}