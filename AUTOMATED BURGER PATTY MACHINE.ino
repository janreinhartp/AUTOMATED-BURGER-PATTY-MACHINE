// Declaration of Libraries
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool refreshScreen = false;
#include <Debounce.h>

byte enterChar[] = {
    B10000,
    B10000,
    B10100,
    B10110,
    B11111,
    B00110,
    B00100,
    B00000};

byte fastChar[] = {
    B00110,
    B01110,
    B00110,
    B00110,
    B01111,
    B00000,
    B00100,
    B01110};
byte slowChar[] = {
    B00011,
    B00111,
    B00011,
    B11011,
    B11011,
    B00000,
    B00100,
    B01110};

// Declaration of LCD Variables
const int numOfMainScreens = 3;
const int numOfSettingScreens = 8;
const int numOfTestMenu = 10;
int currentScreen = 0;

int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
    {"SETTINGS", "ENTER TO EDIT"},
    {"RUN AUTO", "ENTER TO RUN AUTO"},
    {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
    {"DISPENSER 1", "SEC"},
    {"DISPENSER 2", "SEC"},
    {"DISPENSER 3", "SEC"},
    {"PUMP", "SEC"},
    {"MIXING", "MIN"},
    {"DISPENSE FROM MIXER", "MIN"},
    {"DISPENSE MIXTURE", "SEC"},
    {"BACK TO MAIN", "ENTER"}};

String TestMenuScreen[numOfTestMenu] = {
    "DISPENSER 1",
    "DISPENSER 2",
    "DISPENSER 3",
    "PUMP",
    "MIXER",
    "LINEAR",
    "DISPENSER MIXTURE",
    "PNEUMATIC",
    "CONVEYOR",
    "Back to Main Menu"};

int parametersTimer[numOfSettingScreens] = {1, 1, 1, 1, 1, 1,1,1};
int parametersTimerMaxValue[numOfSettingScreens] = {1200, 1200, 1200, 1200, 1200, 1200, 1200, 1200};

int highestValueToDisplay;

bool settingsFlag = false;
bool settingEditTimerFlag = false;
bool runAutoFlag = false;
bool testMenuFlag = false;

int largest()
{
    int firstFour[4];

    for (int i = 0; i < 4; i++) {
        firstFour[i] = parametersTimer[i];
    }

    int maxValue = parametersTimer[0];
    highestValueToDisplay = 0;

    for (int i = 1; i < 4; i++) {
        if (parametersTimer[i] > maxValue) {
            maxValue = parametersTimer[i]; // Update maxValue if a larger value is found
            highestValueToDisplay = i;
        }
    }
}

// Motor
#include "control.h"

// Encoder
#include <ClickEncoder.h>
// Timer 1 for encoder
#include <TimerOne.h>
// Save Function
#include <EEPROMex.h>
#include <AccelStepper.h>

// Declaration of Variables
// Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;
ClickEncoder *encoder;
int16_t last, value;
// Fast Scroll
bool fastScroll = false;

unsigned long previousMillis = 0; // will store last time LED was updated
// constants won't change:
const long interval = 1000; // interval at which to blink (milliseconds)

unsigned long previousMillis2 = 0; // will store last time LED was updated
// constants won't change:
const long interval2 = 100; // interval at which to blink (milliseconds)
unsigned long currentMillis2 = 0;

// Stepper Variables
int ena1 = 22;
int dir1 = 23;
int step1 = 24;
int ena2 = 25;
int dir2 = 26;
int step2 = 27;

long currentPos1 = 0;
long lastPos1 = 0;
long speedStep1 = 8000;
long moveStep1 = 8000;

// MOTOR DECLARATION
Control Mixer(39, 100, 100);
Control Pneumatic(40, 100, 100);
Control Dispenser1(100, 100, 100);
Control Dispenser2(100, 100, 100);
Control Dispenser3(43, 100, 100);
Control Pump(42, 100, 100);
Control Linear(45, 100, 100);
Control Extruder(47, 100, 100);
Control Conveyor(46, 100, 100);
Control DropTimer(100,100,100);



AccelStepper Dispenser1Stepper(AccelStepper::FULL2WIRE, dir1, step1);
AccelStepper Dispenser2Stepper(AccelStepper::FULL2WIRE, dir2, step2);
void setDispenser()
{
    Dispenser1Stepper.setEnablePin(ena1);
    Dispenser1Stepper.setPinsInverted(false, false, false);
    Dispenser1Stepper.setMaxSpeed(speedStep1);
    Dispenser1Stepper.setSpeed(speedStep1);
    Dispenser1Stepper.setAcceleration(speedStep1 * 200);
    Dispenser1Stepper.enableOutputs();

    Dispenser2Stepper.setEnablePin(ena2);
    Dispenser2Stepper.setPinsInverted(false, false, false);
    Dispenser2Stepper.setMaxSpeed(speedStep1);
    Dispenser2Stepper.setSpeed(speedStep1);
    Dispenser2Stepper.setAcceleration(speedStep1 * 200);
    Dispenser2Stepper.enableOutputs();
}



void DisableSteppers()
{
    Dispenser1Stepper.disableOutputs();
    Dispenser2Stepper.disableOutputs();
}
void EnableSteppers()
{
    Dispenser1Stepper.enableOutputs();
    Dispenser2Stepper.enableOutputs();
}

void RunDispenser1Stepper()
{
    if (Dispenser1Stepper.distanceToGo() == 0)
    {
        Dispenser1Stepper.setCurrentPosition(0);
        Dispenser1Stepper.move(moveStep1);
    }
}

void RunDispenser2Stepper()
{
    if (Dispenser2Stepper.distanceToGo() == 0)
    {
        Dispenser2Stepper.setCurrentPosition(0);
        Dispenser2Stepper.move(moveStep1);
    }
}

bool TestRunDispenser1, TestRunDispenser2 = false;
void TestRun(){
    if (TestRunDispenser1 == true)
    {
        Dispenser1Stepper.run();
        RunDispenser1Stepper();
    }
    
    if (TestRunDispenser2 == true)
    {
        Dispenser2Stepper.run();
        RunDispenser2Stepper();
    }
}

int mixTimeAdd = 10;
int dispense1TimeAdd = 20;
int dispense2TimeAdd = 30;
int dispense3TimeAdd = 40;
int pumpTimeAdd = 50;
int linearTimeAdd = 60;
int extruderTimeAdd = 70;

void saveSettings()
{
    EEPROM.updateDouble(dispense1TimeAdd, parametersTimer[0]);
    EEPROM.updateDouble(dispense2TimeAdd, parametersTimer[1]);
    EEPROM.updateDouble(dispense3TimeAdd, parametersTimer[2]);
    EEPROM.updateDouble(pumpTimeAdd, parametersTimer[3]);
    EEPROM.updateDouble(mixTimeAdd, parametersTimer[4]);
    EEPROM.updateDouble(linearTimeAdd, parametersTimer[5]);
    EEPROM.updateDouble(extruderTimeAdd, parametersTimer[6]);

}

void loadSettings()
{
    parametersTimer[0] = EEPROM.readDouble(dispense1TimeAdd);
    parametersTimer[1] = EEPROM.readDouble(dispense2TimeAdd);
    parametersTimer[2] = EEPROM.readDouble(dispense3TimeAdd);
    parametersTimer[3] = EEPROM.readDouble(pumpTimeAdd);
    parametersTimer[4] = EEPROM.readDouble(mixTimeAdd);
    parametersTimer[5] = EEPROM.readDouble(linearTimeAdd);
    parametersTimer[6] = EEPROM.readDouble(extruderTimeAdd);
}

char *secondsToHHMMSS(int total_seconds)
{
    int hours, minutes, seconds;

    hours = total_seconds / 3600;         // Divide by number of seconds in an hour
    total_seconds = total_seconds % 3600; // Get the remaining seconds
    minutes = total_seconds / 60;         // Divide by number of seconds in a minute
    seconds = total_seconds % 60;         // Get the remaining seconds

    // Format the output string
    static char hhmmss_str[7]; // 6 characters for HHMMSS + 1 for null terminator
    sprintf(hhmmss_str, "%02d%02d%02d", hours, minutes, seconds);
    return hhmmss_str;
}

void setTimer()
{
    Dispenser1.setTimer(secondsToHHMMSS(parametersTimer[0]));
    Dispenser2.setTimer(secondsToHHMMSS(parametersTimer[1]));
    Dispenser3.setTimer(secondsToHHMMSS(parametersTimer[2]));
    Pump.setTimer(secondsToHHMMSS(parametersTimer[3]));
    Mixer.setTimer(secondsToHHMMSS(parametersTimer[4] * 60));
    Linear.setTimer(secondsToHHMMSS(parametersTimer[5] * 60));
    Extruder.setTimer(secondsToHHMMSS(parametersTimer[6]));
}

void runAllTest()
{

}

int RunAutoCommand = 0;
int DispensePattyStat = 0;
bool initialMoveConveyor = false;
/*
1 - Dispense Ingre
2 - Mix
3 - Dispense from mixer to Extruder
4 - Patty Making
*/

void runAutoFunc()
{
    ReadSensor();
    switch (RunAutoCommand)
    {
    case 1:
        RunMixAndDispense();
        break;

    case 2:
        RunMix();
        break;

    case 3:
        RunDispenseFromMixer();
        break;

    case 4:
RunExtrudeToMould();
        break;
    default:
        runAutoFlag = false;
        stopAll();
        break;
    }
}

void RunMixAndDispense(){
    Dispenser1.run();
    Dispenser2.run();
    Dispenser3.run();
    Pump.run();
    Mixer.relayOn();
    Dispenser1Stepper.run();
    Dispenser2Stepper.run();
    if (Dispenser1.isTimerCompleted() == true && Dispenser2.isTimerCompleted() == true  && Dispenser3.isTimerCompleted() == true && Pump.isTimerCompleted() == true )
    {
        RunAutoCommand = 2;
        Mixer.start();
    }
    if(Dispenser1.isTimerCompleted() == false){
        RunDispenser1Stepper();
    }
    if(Dispenser2.isTimerCompleted() == false){
        RunDispenser2Stepper();
    }
}

void RunMix(){
    Mixer.run();
    if (Mixer.isTimerCompleted() == true)
    {
        RunAutoCommand = 3;
        Linear.start();
    }
}

void RunDispenseFromMixer(){
    Linear.run();
    Mixer.relayOn();
    if (Linear.isTimerCompleted() == true)
    {
        Mixer.relayOff();
        RunAutoCommand = 4;
        DispensePattyStat = 1;
    }
}

const int pinSen1 = A0;
const int pinSen2 = A1;
Debounce TraySen(pinSen1, 100, true);

bool SenStat1 = false;
void initSensors()
{
    pinMode(pinSen1, INPUT_PULLUP);

}
void ReadSensor()
{
    SenStat1 = TraySen.read();
}



/*
1 - Move Conveyor
2 - Dispense 1
3 - Dispense 2
*/

void RunExtrudeToMould(){
    switch (DispensePattyStat)
    {
    case 1:
        if (initialMoveConveyor == true)
        {
            if (SenStat1 == true)
            {
                Conveyor.relayOn();
            }
            else
            {
                initialMoveConveyor = false;
            }
        }
        else
        {
            if (SenStat1 == true)
            {
                Conveyor.relayOff();
                Extruder.start();
                DispensePattyStat = 2;
            }
            else
            {
                Conveyor.relayOn();
            }
        }
        break;
    case 2:
        RunDispense1();
        break;
    case 3:
        RunDispense2();
        break;
    default:
        break;
    }
}

void RunDispense1()
{
    Extruder.run();
    if (Extruder.isTimerCompleted() == true)
    {
        Pneumatic.relayOn();
        delay(2000);
        DispensePattyStat = 3;
        Extruder.start();
    }
}

void RunDispense2()
{
    Extruder.run();
    if (Extruder.isTimerCompleted() == true)
    {
        Pneumatic.relayOff();
        delay(2000);
        initialMoveConveyor = true;
        DispensePattyStat = 1;
    }
}

void stopAll()
{
    Mixer.stop();
    Pneumatic.stop();
    Dispenser1.stop();
    Dispenser2.stop();
    Dispenser3.stop();
    Pump.stop();
    Linear.stop();
    Extruder.stop();
    Conveyor.stop();
    DropTimer.stop();
    RunAutoCommand = 0;
    runAutoFlag = false;
    DisableSteppers();
}


// Functions for Rotary Encoder
void timerIsr()
{
    encoder->service();
}

void readRotaryEncoder()
{
    value += encoder->getValue();

    if (value / 2 > last)
    {
        last = value / 2;
        down = true;
        delay(100);
    }
    else if (value / 2 < last)
    {
        last = value / 2;
        up = true;
        delay(100);
    }
}

void readButtonEncoder()
{
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open)
    { // Open Bracket for Click
        switch (b)
        { // Open Bracket for Double Click
        case ClickEncoder::Clicked:
            middle = true;
            break;
        }
    }
}

void inputCommands()
{
    // LCD Change Function and Values
    //  To Right Rotary
    if (up == 1)
    {
        up = false;
        refreshScreen = true;
        if (settingsFlag == true)
        {
            if (settingEditTimerFlag == true)
            {
                if (parametersTimer[currentSettingScreen] >= parametersTimerMaxValue[currentSettingScreen] - 1)
                {
                    parametersTimer[currentSettingScreen] = parametersTimerMaxValue[currentSettingScreen];
                }
                else
                {
                        parametersTimer[currentSettingScreen] += 1;
                }
            }
            else
            {
                if (currentSettingScreen == numOfSettingScreens - 1)
                {
                    currentSettingScreen = 0;
                }
                else
                {
                    currentSettingScreen++;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == numOfTestMenu - 1)
            {
                currentTestMenuScreen = 0;
            }
            else
            {
                currentTestMenuScreen++;
            }
        }
        else
        {
            if (currentScreen == numOfMainScreens - 1)
            {
                currentScreen = 0;
            }
            else
            {
                currentScreen++;
            }
        }
    }

    // To Left Rotary
    if (down == 1)
    {
        down = false;
        refreshScreen = true;
        if (settingsFlag == true)
        {
            if (settingEditTimerFlag == true)
            {
                if (parametersTimer[currentSettingScreen] <= 0)
                {
                    parametersTimer[currentSettingScreen] = 0;
                }
                else
                {
                        parametersTimer[currentSettingScreen] -= 1;
                }
            }
            else
            {
                if (currentSettingScreen <= 0)
                {
                    currentSettingScreen = numOfSettingScreens - 1;
                }
                else
                {
                    currentSettingScreen--;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen <= 0)
            {
                currentTestMenuScreen = numOfTestMenu - 1;
            }
            else
            {
                currentTestMenuScreen--;
            }
        }
        else
        {
            if (currentScreen == 0)
            {
                currentScreen = numOfMainScreens - 1;
            }
            else
            {
                currentScreen--;
            }
        }
    }

    // Rotary Button Press
    if (middle == 1)
    {
        middle = false;
        refreshScreen = 1;
        if (currentScreen == 0 && settingsFlag == true)
        {
            if (currentSettingScreen == numOfSettingScreens - 1)
            {
                settingsFlag = false;
                saveSettings();
                loadSettings();
                setTimer();
                largest();
                currentSettingScreen = 0;
            }
            else
            {
                if (settingEditTimerFlag == true)
                {
                    settingEditTimerFlag = false;
                }
                else
                {
                    settingEditTimerFlag = true;
                }
            }
        }
        else if (runAutoFlag == true)
        {
            stopAll();
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == numOfTestMenu - 1)
            {
                currentTestMenuScreen = 0;
                testMenuFlag = false;
            }
            else if (currentTestMenuScreen == 0)
            {
                if(TestRunDispenser1 == false){
                    TestRunDispenser1 = true;
                    Dispenser1Stepper.enableOutputs();
                    Dispenser1Stepper.move(moveStep1);
                }else{
                    TestRunDispenser1 = false;
                    Dispenser1Stepper.enableOutputs();
                }
            } 
            else if (currentTestMenuScreen == 1)
            {
                if(TestRunDispenser2 == false){
                    TestRunDispenser2 = true;
                    Dispenser2Stepper.enableOutputs();
                    Dispenser2Stepper.move(moveStep1);
                }else{
                    TestRunDispenser2 = false;
                    Dispenser2Stepper.enableOutputs();
                }
            }
            else if (currentTestMenuScreen == 2)
            {
                if (Dispenser3.getMotorState() == true)
                {
                    Dispenser3.relayOff();
                }
                else
                {
                    Dispenser3.relayOn();
                }
            }
            else if (currentTestMenuScreen == 3)
            {
                if (Pump.getMotorState() == true)
                {
                    Pump.relayOff();
                }
                else
                {
                    Pump.relayOn();
                }
            }
            else if (currentTestMenuScreen == 4)
            {
                if (Mixer.getMotorState() == true)
                {
                    Mixer.relayOff();
                }
                else
                {
                    Mixer.relayOn();
                }
            }
            else if (currentTestMenuScreen == 5)
            {
                if (Linear.getMotorState() == true)
                {
                    Linear.relayOff();
                }
                else
                {
                    Linear.relayOn();
                }
            }
            else if (currentTestMenuScreen == 6)
            {
                if (Extruder.getMotorState() == true)
                {
                    Extruder.relayOff();
                }
                else
                {
                    Extruder.relayOn();
                }
            }
            else if (currentTestMenuScreen == 7)
            {
                if (Pneumatic.getMotorState() == true)
                {
                    Pneumatic.relayOff();
                }
                else
                {
                    Pneumatic.relayOn();
                }
            }
            else if (currentTestMenuScreen == 8)
            {
                if (Conveyor.getMotorState() == true)
                {
                    Conveyor.relayOff();
                }
                else
                {
                    Conveyor.relayOn();
                }
            }
        }
        else
        {
            if (currentScreen == 0)
            {
                settingsFlag = true;
            }
            else if (currentScreen == 1)
            {
                runAutoFlag = true;
                // Insert Commands for Run Auto
                Pump.start();
                Dispenser1.start();
                Dispenser2.start();
                Dispenser3.start();
                EnableSteppers();
                RunAutoCommand = 1;
                refreshScreen = 1;
            }
            else if (currentScreen == 2)
            {
                testMenuFlag = true;
            }
        }
    }
}

void PrintRunAuto(String job, char *time)
{
    lcd.clear();
    lcd.print("Running Auto");
    lcd.setCursor(0, 1);
    lcd.print("Status: ");
    lcd.setCursor(0, 2);
    lcd.print(job);
    lcd.setCursor(0, 3);
    lcd.print("Timer: ");
    lcd.setCursor(7, 3);
    lcd.print(time);
}

void printScreen()
{

    if (settingsFlag == true)
    {
        lcd.clear();
        lcd.print(settings[currentSettingScreen][0]);
        lcd.setCursor(0, 1);
        if (currentSettingScreen == numOfSettingScreens - 1)
        {
            lcd.setCursor(0, 3);
            lcd.write(0);
            lcd.setCursor(2, 3);
            lcd.print("Click to Save All");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print(parametersTimer[currentSettingScreen]);
            lcd.print(" ");
            lcd.print(settings[currentSettingScreen][1]);
            lcd.setCursor(0, 3);
            lcd.write(0);
            lcd.setCursor(2, 3);
            if (settingEditTimerFlag == false)
            {
                lcd.print("ENTER TO EDIT");
            }
            else
            {
                lcd.print("ENTER TO SAVE");
            }
            lcd.setCursor(19, 3);
        }
    }
    else if (runAutoFlag == true)
    {
        if (runAutoFlag == true && RunAutoCommand == 1)
        {
            if(highestValueToDisplay == 0){
                PrintRunAuto("DISPENSING INGRE", Dispenser1.getTimeRemaining());
            }else if(highestValueToDisplay == 1){
                PrintRunAuto("DISPENSING INGRE", Dispenser2.getTimeRemaining());
            }else if(highestValueToDisplay == 2){
                PrintRunAuto("DISPENSING INGRE", Dispenser3.getTimeRemaining());
            }else if(highestValueToDisplay == 3){
                PrintRunAuto("DISPENSING INGRE", Pump.getTimeRemaining());
            }
        }
        else if (runAutoFlag == true && RunAutoCommand == 2)
        {
            PrintRunAuto("MIXING", Mixer.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 3)
        {
            PrintRunAuto("DISPENSE TO EXTRUDE", Linear.getTimeRemaining());
        }
        else if(runAutoFlag == true && RunAutoCommand == 4){
            switch (DispensePattyStat)
            {
            case 1:
                PrintRunAuto("CONVEYOR MOVING", "N/A");
                break;
            case 2:
                PrintRunAuto("EXTRUDING 1", Extruder.getTimeRemaining());
                break;
            case 3:
                PrintRunAuto("EXTRUDING 2", Extruder.getTimeRemaining());
                break;
            default:
                break;
            }
        }
    }
    else if (testMenuFlag == true)
    {
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);

        if (currentTestMenuScreen == numOfTestMenu - 1)
        {
            lcd.setCursor(0, 3);
            lcd.print("Click to Exit Test");
        }
        else
        {
            lcd.setCursor(0, 3);
            lcd.print("Click to Run Test");
        }
    }
    else
    {
        lcd.clear();
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        lcd.print(screens[currentScreen][1]);
        lcd.setCursor(19, 3);
        lcd.print(highestValueToDisplay);
        refreshScreen = false;
    }
}

void setupJumper()
{
}

void setup()
{
    // Encoder Setup
    encoder = new ClickEncoder(4, 2, 3); // Actual
    // encoder = new ClickEncoder(3, 4, 2); // TestBench
    encoder->setAccelerationEnabled(false);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
    last = encoder->getValue();

    // LCD Setup
    lcd.init();
    lcd.createChar(0, enterChar);
    lcd.createChar(1, fastChar);
    lcd.createChar(2, slowChar);
    lcd.clear();
    lcd.backlight();
    
    Serial.begin(9600);

     //saveSettings(); // Disable upon Initialiaze
    setDispenser();
    DisableSteppers();
    loadSettings();
    setTimer();
    largest();
    refreshScreen = true;
}

void loop()
{
    readRotaryEncoder();
    readButtonEncoder();
    inputCommands();

    if (refreshScreen == true)
    {
        printScreen();
        refreshScreen = false;
    }

    if (runAutoFlag == true)
    {
        runAutoFunc();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            Serial.print("Current RunAutoCommand: ");
            Serial.println(RunAutoCommand);
            Serial.print("Current DispensePattyStat: ");
            Serial.println(DispensePattyStat);
            refreshScreen = true;
        }
    }

    if (testMenuFlag == true)
    {
        TestRun();
    }
}