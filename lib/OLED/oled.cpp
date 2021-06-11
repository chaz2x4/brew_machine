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
            if(this->isEditable) this->incrementTemp();
            else this->changeMode();
        }
        else if(buttonState[0] == HIGH && lastButtonState[0] == LOW) { downTime = -1 ; }
        lastButtonState[0] = buttonState[0];

        //Decrease temperature when editable on button B
        if(buttonState[1] == LOW && lastButtonState[1] == HIGH) {
            downTime = millis();
            timeLastButton = millis();
            if(this->isEditable) this->decrementTemp();
            else this->changeMode();
        }
        else if(buttonState[1] == HIGH && lastButtonState[1] == LOW) { downTime = -1 ; }
        lastButtonState[1] = buttonState[1];

        //Change mode if button C is pressed
        if(buttonState[2] == LOW && lastButtonState[2] == HIGH) {
            downTime = millis();
            timeLastButton = millis();
            if(this->isEditable) this->isEditable = false;
        }
        else if(buttonState[2] == HIGH && lastButtonState[2] == LOW) {
            downTime = -1;
        }
        if(buttonState[2] == LOW && this->getCurrentMode() == brew && (millis() - downTime) >= TRIGGER_TIME) {
            this->isEditable = true;
        }
        lastButtonState[2] = buttonState[2];
    }
}

void OLED::wait (int delay, char* text1, float var1, char* text2, float var2){
    display.clearDisplay();
    if(flash) display.printf(text1, var1);
    else display.printf(text2, var2);
    
    if((millis() - lastTime) >= delay) {
        lastTime = millis();
        flash = !flash;
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
    this->update();
    if(timedout()) return;
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 0);
    mode currentMode = this->getCurrentMode();
    if(currentMode == brew) {
        if(this->isEditable) {
            wait(500, (char *)("Set Brew: %.1f C"), this->getTargetTemp(), (char *)("Set Brew: "), 0x0);
        }
        else {
            wait(2000, (char *)("Target Temp: %.1f C\n"), this->getTargetTemp(), (char *)("Temp: \n %.1f C\n"), this->getActualTemp());
        }
    }
    if(currentMode == steam) {
        wait(1000, (char *)("Target Temp: %.1f C\n"), this->getTargetSteamTemp(), (char *)("Steam Temp: %.1f C\n"), this->getActualTemp());
    }
    display.display();
}

void OLED::changeMode(){
    display.clearDisplay();
    if(this->getCurrentMode() == brew) this->setMode(steam);
    else this->setMode(brew);
}