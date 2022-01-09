#include <header.h>
#include <unity.h>
#include <ArduinoJson.h> 

GCP brew_machine;
OLED screen;
DynamicJsonDocument savedTargetTunings(1024);
double savedTargetBrewTemp;
double savedTargetSteamTemp;
double savedTargetOffset;

void setUp(){
    String savedTuningsString;
    savedTargetBrewTemp = brew_machine.getTargetTemp("brew");
    savedTargetSteamTemp = brew_machine.getTargetTemp("steam");
    savedTargetOffset = brew_machine.getTempOffset();
    deserializeJson(savedTargetTunings, brew_machine.getTunings());
}

void tearDown(){
    brew_machine.setTargetTemp("brew", savedTargetBrewTemp);
    brew_machine.setTargetTemp("steam", savedTargetSteamTemp);
    brew_machine.setTempOffset(savedTargetOffset);
    brew_machine.setTunings(savedTargetTunings["kp"], savedTargetTunings["ki"], savedTargetTunings["kd"]);
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
    double expectedTunings[3] = {11, 11, 11};
    brew_machine.setTunings(11, 11, 11);
    for(int i=0; i<3 ; i++) {
        EEPROM.get(TUNING_ADDRESS + i*8, actualTunings[i]);
    }
    TEST_ASSERT_EQUAL(expectedTunings, actualTunings);
}

void test_function_gcp_output(){

}

void setup(){
    delay(2000);
    brew_machine.start();
    screen.start(&brew_machine);
    
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
    RUN_TEST(test_function_gcp_output);
    UNITY_END();
}


void loop(){

}