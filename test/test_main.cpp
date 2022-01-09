#include <header.h>
#include <unity.h>
#include <ArduinoJson.h> 

GCP brew_machine;
OLED screen;
double savedTargetBrewTemp;
double savedTargetSteamTemp;
double savedTargetOffset;
double savedTunings[3];
ulong timerStartTime;

int currentLoop = 0;
int maxLoops = 10;

void loadParameters(){
    EEPROM.get(BREW_TEMP_ADDRESS, savedTargetBrewTemp);
    EEPROM.get(STEAM_TEMP_ADDRESS, savedTargetSteamTemp);
    EEPROM.get(OFFSET_ADDRESS, savedTargetOffset);
    for(int i=0;i<3;i++) {
        EEPROM.get(TUNING_ADDRESS + i*8, savedTunings[i]);
    }
}

void saveParameters(){
    brew_machine.setTargetTemp("brew", savedTargetBrewTemp);
    brew_machine.setTargetTemp("steam", savedTargetSteamTemp);
    brew_machine.setTempOffset(savedTargetOffset);
    brew_machine.setTunings(savedTunings[0], savedTunings[1], savedTunings[2]);
}

void test_function_gcp_set_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    TEST_ASSERT_EQUAL(95, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_set_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    TEST_ASSERT_EQUAL(150, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_set_offset(){
    brew_machine.setTempOffset(-10);
    TEST_ASSERT_EQUAL(-10, brew_machine.getTempOffset());
}

void test_function_gcp_increment_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    brew_machine.incrementTemp("brew");
    TEST_ASSERT_EQUAL(95.5, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_increment_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    brew_machine.incrementTemp("steam");
    TEST_ASSERT_EQUAL(151, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_decrement_brew_temp(){
    brew_machine.setTargetTemp("brew", 95);
    brew_machine.decrementTemp("brew");
    TEST_ASSERT_EQUAL(94.5, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_decrement_steam_temp(){
    brew_machine.setTargetTemp("steam", 150);
    brew_machine.decrementTemp("steam");
    TEST_ASSERT_EQUAL(149, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_brew_max_1(){
    brew_machine.setTargetTemp("brew", 162);
    TEST_ASSERT_EQUAL(115, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_brew_max_2(){
    brew_machine.setTargetTemp("brew", 162);
    brew_machine.incrementTemp("brew");
    TEST_ASSERT_EQUAL(115, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_brew_min_1(){
    brew_machine.setTargetTemp("brew", 0);
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_brew_min_2(){
    brew_machine.setTargetTemp("brew", 0);
    brew_machine.decrementTemp("brew");
    TEST_ASSERT_EQUAL(85, brew_machine.getTargetTemp("brew"));
}

void test_function_gcp_steam_max_1(){
    brew_machine.setTargetTemp("steam", 162);
    TEST_ASSERT_EQUAL(160, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_steam_max_2(){
    brew_machine.setTargetTemp("steam", 162);
    brew_machine.incrementTemp("steam");
    TEST_ASSERT_EQUAL(160, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_steam_min_1(){
    brew_machine.setTargetTemp("steam", 0);
    TEST_ASSERT_EQUAL(140, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_steam_min_2(){
    brew_machine.setTargetTemp("steam", 0);
    brew_machine.decrementTemp("steam");
    TEST_ASSERT_EQUAL(140, brew_machine.getTargetTemp("steam"));
}

void test_function_gcp_offset_max(){
    brew_machine.setTempOffset(20);
    TEST_ASSERT_EQUAL(15, brew_machine.getTempOffset());
}

void test_function_gcp_offset_min(){
    brew_machine.setTempOffset(-20);
    TEST_ASSERT_EQUAL(-15, brew_machine.getTempOffset());
}

void test_function_gcp_current_temp(){
    double temp = brew_machine.getActualTemp();
    double offset = brew_machine.getTempOffset();
    TEST_ASSERT_EQUAL(temp + offset, brew_machine.getCurrentTemp());
}

void test_function_gcp_set_tunings(){
    brew_machine.setTunings(10, 10, 10);
    TEST_ASSERT_EQUAL_STRING("{ \"kp\": 10.00, \"ki\": 10.00, \"kd\": 10.00 }", brew_machine.getTunings().c_str());
}

void test_function_gcp_eeprom_brew(){
    double temp;
    brew_machine.setTargetTemp("brew", 96);
    EEPROM.get(BREW_TEMP_ADDRESS, temp);
    TEST_ASSERT_EQUAL(96, temp);
}

void test_function_gcp_eeprom_steam(){
    double temp;
    brew_machine.setTargetTemp("steam", 152);
    EEPROM.get(STEAM_TEMP_ADDRESS, temp);
    TEST_ASSERT_EQUAL(152, temp);
}

void test_function_gcp_eeprom_offset(){
    double temp;
    brew_machine.setTempOffset(-11);
    EEPROM.get(OFFSET_ADDRESS, temp);
    TEST_ASSERT_EQUAL(-11, temp);
}

void test_function_gcp_eeprom_tunings(){
    double actualTunings[3];
    double expectedTunings[3] = {11, 12, 13};
    brew_machine.setTunings(11, 12, 13);
    for(int i=0; i<3 ; i++) {
        EEPROM.get(TUNING_ADDRESS + i*8, actualTunings[i]);
        TEST_ASSERT_EQUAL(expectedTunings[i], actualTunings[i]);
    }
}

void test_function_oled_increment_brew(){
    String currentMode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp("brew");
    if(currentMode == "brew") {
        screen.incrementTemp();
        TEST_ASSERT_EQUAL(target + 0.5, brew_machine.getTargetTemp("brew"));
    }
    else {
        screen.changeMode();
        test_function_oled_increment_brew();
    }
}

void test_function_oled_decrement_brew(){
    String currentMode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp("brew");
    if(currentMode == "brew") {
        screen.decrementTemp();
        TEST_ASSERT_EQUAL(target - 0.5, brew_machine.getTargetTemp("brew"));
    }
    else {
        screen.changeMode();
        test_function_oled_decrement_brew();
    }
}

void test_function_oled_increment_steam(){
    String currentMode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp("steam");
    if(currentMode == "steam") {
        screen.incrementTemp();
        TEST_ASSERT_EQUAL(target + 1, brew_machine.getTargetTemp("steam"));
    }
    else {
        screen.changeMode();
        test_function_oled_increment_steam();
    }
}

void test_function_oled_decrement_steam(){
    String currentMode = screen.getCurrentMode();
    double target = brew_machine.getTargetTemp("steam");
    if(currentMode == "steam") {
        screen.decrementTemp();
        TEST_ASSERT_EQUAL(target - 1, brew_machine.getTargetTemp("steam"));
    }
    else {
        screen.changeMode();
        test_function_oled_decrement_steam();
    }
}

void test_function_gcp_timer_running(){
    double now = millis();
    double timer = brew_machine.getCurrentTimer(now);
    TEST_ASSERT_EQUAL((now - timerStartTime) / 1000, timer);
}

void test_function_gcp_timer_not_running(){
    double now = millis();
    double timer = brew_machine.getCurrentTimer(now);
    TEST_ASSERT_EQUAL(0, timer);
}

void setup(){
    delay(2000);
    brew_machine.start();
    screen.start(&brew_machine);
    EEPROM.begin(512);
    loadParameters();    

    UNITY_BEGIN();
    RUN_TEST(test_function_gcp_set_brew_temp);
    RUN_TEST(test_function_gcp_set_steam_temp);
    RUN_TEST(test_function_gcp_set_offset);
    RUN_TEST(test_function_gcp_increment_brew_temp);
    RUN_TEST(test_function_gcp_decrement_brew_temp);
    RUN_TEST(test_function_gcp_increment_steam_temp);
    RUN_TEST(test_function_gcp_decrement_steam_temp);
    RUN_TEST(test_function_gcp_brew_max_1);
    RUN_TEST(test_function_gcp_brew_min_1);
    RUN_TEST(test_function_gcp_steam_max_1);
    RUN_TEST(test_function_gcp_steam_min_1);
    RUN_TEST(test_function_gcp_brew_max_2);
    RUN_TEST(test_function_gcp_brew_min_2);
    RUN_TEST(test_function_gcp_steam_max_2);
    RUN_TEST(test_function_gcp_steam_min_2);
    RUN_TEST(test_function_gcp_offset_max);
    RUN_TEST(test_function_gcp_offset_min);
    RUN_TEST(test_function_gcp_current_temp);
    RUN_TEST(test_function_gcp_set_tunings);
    RUN_TEST(test_function_gcp_eeprom_brew);
    RUN_TEST(test_function_gcp_eeprom_steam);
    RUN_TEST(test_function_gcp_eeprom_offset);
    RUN_TEST(test_function_gcp_eeprom_tunings);
    // RUN_TEST(test_function_gcp_output);

    RUN_TEST(test_function_oled_increment_brew);
    RUN_TEST(test_function_oled_increment_steam);
    // RUN_TEST(test_function_oled_increment_offset);
    RUN_TEST(test_function_oled_decrement_brew);
    RUN_TEST(test_function_oled_decrement_steam);
    // RUN_TEST(test_function_oled_decrement_offset);

    timerStartTime = millis();
    brew_machine.startTimer(timerStartTime);
}

void loop(){
    if(currentLoop < maxLoops) {
        RUN_TEST(test_function_gcp_timer_running);
        delay(1000);
        currentLoop++;
    }
    else {
        brew_machine.stopTimer();
        RUN_TEST(test_function_gcp_timer_not_running);
        UNITY_END();
        saveParameters();
    }
}