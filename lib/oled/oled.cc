#include "oled.h"

OLED::OLED()
: display(Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire))
, current_mode(BREW)
, time_last_button(-1)
, last_time(-1)
, downtime(-1)
, is_editable(false)
, flash(false)
, button_state{HIGH, HIGH, HIGH}
, last_button_state{HIGH, HIGH, HIGH}
, kScreenTimeout(900000)
, kTriggerTime(1000)
{}

OLED::~OLED(){
    delete gcp;
}

void OLED::start(GCP* brew_machine){
    display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
    display.clearDisplay();
    display.dim(true);
    display.display();

    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    pinMode(BUTTON_C, INPUT_PULLUP);
    time_last_button = millis();

    gcp = brew_machine;
}

void OLED::eventListener(){
    button_state[0] = digitalRead(BUTTON_A);
    button_state[1] = digitalRead(BUTTON_B);
    button_state[2] = digitalRead(BUTTON_C);

    ulong now = millis();

    if(checkedIfTimedout()) { //on any button press immediately turn screen on, but dont do anything else
        if((button_state[0] == LOW && last_button_state[0] == HIGH) ||
            (button_state[1] == LOW && last_button_state[1] == HIGH) ||
            (button_state[2] == LOW && last_button_state[2] == HIGH)) {
                time_last_button = now;
                last_button_state[0] = button_state[0];
                last_button_state[1] = button_state[1];
                last_button_state[2] = button_state[2];
        }
    }
    else {
        //Increase temperature when editable on button A
        if(button_state[0] == LOW && last_button_state[0] == HIGH) {
            downtime = now;
            time_last_button = now;
            if(this->is_editable) this->incrementTemp();
            else this->changeMode();
        }
        else if(button_state[0] == HIGH && last_button_state[0] == LOW) downtime = -1 ;
        last_button_state[0] = button_state[0];

        //Decrease temperature when editable on button B
        if(button_state[1] == LOW && last_button_state[1] == HIGH) {
            downtime = now;
            time_last_button = now;
            if(this->is_editable) this->decrementTemp();
            else this->changeMode();
        }
        else if(button_state[1] == HIGH && last_button_state[1] == LOW) downtime = -1 ;
        last_button_state[1] = button_state[1];

        //Change mode if button C is pressed
        if(button_state[2] == LOW && last_button_state[2] == HIGH) {
            downtime = now;
            time_last_button = now;
            if(this->is_editable) {
                this->is_editable = false;
                last_time = now;
                flash = true;
            }
        }
        else if(button_state[2] == HIGH && last_button_state[2] == LOW) downtime = -1; 
        if(button_state[2] == LOW && (now - downtime) >= kTriggerTime) this->is_editable = true;
        last_button_state[2] = button_state[2];
        display.clearDisplay();
    }
}

bool OLED::checkedIfTimedout(){
    ulong now = millis();
    if((now - time_last_button) >= kScreenTimeout) {
        this->is_editable = false;
        display.clearDisplay();
        display.display();
        return true;
    }
    return false;
}

void OLED::refresh(ulong real_time){
    this->real_time = real_time;
    this->eventListener();
    if(checkedIfTimedout()) return;
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 0);
    display.clearDisplay();

    ulong wait;
    ulong now = millis();
    char* modeTitle = "Brew";
    double target_temp = gcp->getTargetTemp(BREW);
    double current_temp = gcp->getCurrentTemp();
    if(this->current_mode == STEAM) {
        modeTitle = "Steam";
        target_temp = gcp->getTargetTemp(STEAM);
    }
    if(this->is_editable) {
        if(flash) display.printf("Set %s\n %#.1f °", modeTitle, target_temp);
        else display.printf("Set %s", modeTitle);
        wait = 500;
    }
    else {
        if(flash) display.printf("%sing\n %#0.1f °", modeTitle, target_temp);
        else display.printf("Temp:\n %#.1f C", current_temp);
        wait = 2000;
    }

    if((now - last_time) > wait) {
        last_time = now;
        flash = !flash;
    }
    display.display();
}

void OLED::changeMode(){
    display.clearDisplay();
    last_time = millis();
    flash = true;
    this->current_mode = this->current_mode == BREW ? STEAM : BREW;
}

TempMode OLED::getCurrentMode(){
    return this->current_mode;;
}

void OLED::incrementTemp(){
    gcp->incrementTemp(this->current_mode);
}

void OLED::decrementTemp(){
    gcp->decrementTemp(this->current_mode);
}