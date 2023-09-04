// Declaration of Libraries
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool refreshScreen = false;

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
const int numOfSettingScreens = 9;
const int numOfTestMenu = 10;
int currentScreen = 0;

int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
    {"SETTINGS", "ENTER TO EDIT"},
    {"RUN AUTO", "ENTER TO RUN AUTO"},
    {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
    {"DISPENSE BUKO", "MIN"},
    {"SUGAR DISPENSER", "MIN"},
    {"PUMP", "MIN"},
    {"CARAMELIZE SUGAR", "MIN"},
    {"COOKING", "MIN"},
    {"COOLING", "MIN"},
    {"DISPENSE COOKER", "MIN"},
    {"SLICE INTERVAL TIME", "SEC"},
    {"SAVE SETTINGS", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
    "BUKO DISPENSER",
    "SUGAR DISPENSER",
    "PUMP",
    "MIXER COOKER",
    "HEATER",
    "LINEAR",
    "EXTRUDER",
    "CUTTER",
    "CONVEYOR",
    "Back to Main Menu"};

double parametersTimer[numOfSettingScreens] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
double parametersTimerMaxValue[numOfSettingScreens] = {1200, 1200, 1200, 1200, 1200, 1200, 1200, 1200, 1200};

bool settingsFlag = false;
bool settingEditTimerFlag = false;
bool runAutoFlag = false;
bool testMenuFlag = false;

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
int ena1 = A10;
int dir1 = A11;
int step1 = A12;
long currentPos1 = 0;
long lastPos1 = 0;
long speedStep1 = 8000;
long moveStep1 = 8000;

// MOTOR DECLARATION
Control BukoDispenser(38, 100, 100);
Control SugarTimer(100, 100, 100);
Control Pump(46, 100, 100);

Control Mixer(47, 100, 100);
Control Heater(37, 35, 100);

Control CookingTimer1(100, 100, 100);
Control CookingTimer2(100, 100, 100);
Control CoolingTimer(100, 100, 100);

Control Linear(36, 100, 100);

Control Extruder(41, 100, 100);
Control Conveyor(42, 100, 100);
Control TimerCutterDelay(101, 101, 101);
Control Cutter(34, 100, 100);

AccelStepper SugarDispenser(AccelStepper::FULL2WIRE, dir1, step1);
void setDispenser()
{
    SugarDispenser.setEnablePin(ena1);
    SugarDispenser.setPinsInverted(false, false, false);
    SugarDispenser.setMaxSpeed(speedStep1);
    SugarDispenser.setSpeed(speedStep1);
    SugarDispenser.setAcceleration(speedStep1 * 200);
    SugarDispenser.enableOutputs();
    lastPos1 = SugarDispenser.currentPosition();
}

void DisableSteppers()
{
    SugarDispenser.disableOutputs();
}
void EnableSteppers()
{
    SugarDispenser.enableOutputs();
}

void RunSugarDispenser()
{
    if (SugarDispenser.distanceToGo() == 0)
    {
        SugarDispenser.setCurrentPosition(0);
        SugarDispenser.move(moveStep1);
    }
}

int dispenseBukoTimeAdd = 10;
int sugarDispenseTimeAdd = 20;
int pumpTimeAdd = 30;
int cookingTime1TimeAdd = 40;
int cookingTime2TimeAdd = 50;
int coolingTimeAdd = 60;
int dispenseCookerTimeAdd = 70;
int sliceIntervalTimeAdd = 80;

void saveSettings()
{
    EEPROM.updateDouble(dispenseBukoTimeAdd, parametersTimer[0]);
    EEPROM.updateDouble(sugarDispenseTimeAdd, parametersTimer[1]);
    EEPROM.updateDouble(pumpTimeAdd, parametersTimer[2]);
    EEPROM.updateDouble(cookingTime1TimeAdd, parametersTimer[3]);
    EEPROM.updateDouble(cookingTime2TimeAdd, parametersTimer[4]);
    EEPROM.updateDouble(coolingTimeAdd, parametersTimer[5]);
    EEPROM.updateDouble(dispenseCookerTimeAdd, parametersTimer[6]);
    EEPROM.updateDouble(sliceIntervalTimeAdd, parametersTimer[7]);
}

void loadSettings()
{
    parametersTimer[0] = EEPROM.readDouble(dispenseBukoTimeAdd);
    parametersTimer[1] = EEPROM.readDouble(sugarDispenseTimeAdd);
    parametersTimer[2] = EEPROM.readDouble(pumpTimeAdd);
    parametersTimer[3] = EEPROM.readDouble(cookingTime1TimeAdd);
    parametersTimer[4] = EEPROM.readDouble(cookingTime2TimeAdd);
    parametersTimer[5] = EEPROM.readDouble(coolingTimeAdd);
    parametersTimer[6] = EEPROM.readDouble(dispenseCookerTimeAdd);
    parametersTimer[7] = EEPROM.readDouble(sliceIntervalTimeAdd);
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
    BukoDispenser.setTimer(secondsToHHMMSS(parametersTimer[0]*60));
    SugarTimer.setTimer(secondsToHHMMSS(parametersTimer[1]*60));
    Pump.setTimer(secondsToHHMMSS(parametersTimer[2]*60));
    CookingTimer1.setTimer(secondsToHHMMSS(parametersTimer[3] * 60));
    CookingTimer2.setTimer(secondsToHHMMSS(parametersTimer[4] * 60));
    CoolingTimer.setTimer(secondsToHHMMSS(parametersTimer[5] * 60));
    Linear.setTimer(secondsToHHMMSS(parametersTimer[6] * 60));
    TimerCutterDelay.setTimer(secondsToHHMMSS(parametersTimer[7]));
}

bool testRunCut, testSugarDispenser = false;
void runAllTest()
{

    if (testRunCut == true)
    {
        RunCut();
    }

    if (testSugarDispenser == true)
    {
        SugarDispenser.run();
        RunSugarDispenser();
    }
}

int RunAutoCommand = 0;
int cutStat = 0;

/*
1 - Dispense Sugar (Heat and Mixer On)
2 - Dispense Water (Heat and Mixer On)
3 - Cook 1 (Heat and Mixer On)
4 - Dispense Buko (Heat and Mixer On)
5 - Cook 2 (Heat and Mixer On)
6 - Cooling
7 - Dispense to Extruder (Mixer On)
8 - Extrude
*/

void runAutoFunc()
{
    switch (RunAutoCommand)
    {
    case 1:
        RunDispenseWater();

        break;

    case 2:
        RunDispenseSugar();
        break;

    case 3:
        RunCook1();
        break;

    case 4:
        RunDispenseBuko();
        break;

    case 5:
        RunCook2();
        break;

    case 6:
        RunCooling();
        break;

    case 7:
        RunDispenseToExtruder();
        break;

    case 8:
        RunExtrude();
        break;

    default:
        runAutoFlag = false;
        stopAll();
        break;
    }
}

void RunDispenseSugar()
{
    SugarTimer.run();
    Heater.relayOn();
  //  Mixer.relayOn();
    SugarDispenser.run();
    if (SugarTimer.isTimerCompleted() == true)
    {
        RunAutoCommand = 3;
        CookingTimer1.start();
        DisableSteppers();
    }
    else
    {
        RunSugarDispenser();
    }
}

void RunDispenseWater()
{
    Pump.run();
    Heater.relayOn();
    //Mixer.relayOn();
    if (Pump.isTimerCompleted() == true)
    {
        RunAutoCommand = 2;
        SugarTimer.start();
        EnableSteppers();
    }
}

void RunCook1()
{
    CookingTimer1.run();
    Heater.relayOn();
   // Mixer.relayOn();
    if (CookingTimer1.isTimerCompleted() == true)
    {
        RunAutoCommand = 4;
        BukoDispenser.start();
    }
}

void RunDispenseBuko()
{
    BukoDispenser.run();
    Heater.relayOn();
   // Mixer.relayOn();
    if (BukoDispenser.isTimerCompleted() == true)
    {
        RunAutoCommand = 5;
        CookingTimer2.start();
    }
}

void RunCook2()
{
    CookingTimer2.run();
    Heater.relayOn();
   // Mixer.relayOn();
    if (CookingTimer2.isTimerCompleted() == true)
    {
        RunAutoCommand = 6;
        CoolingTimer.start();
    }
}

void RunCooling()
{
    CoolingTimer.run();
    Heater.relayOff();
   // Mixer.relayOn();
    if (CoolingTimer.isTimerCompleted() == true)
    {
        RunAutoCommand = 7;
        Linear.start();
    }
}

void RunDispenseToExtruder()
{
    Linear.run();
    Mixer.relayOn();
    if (Linear.isTimerCompleted() == true)
    {
        RunAutoCommand = 8;
        Mixer.relayOff();
        TimerCutterDelay.start();
    }
}

void stopAll()
{

    RunAutoCommand = 0;
    runAutoFlag = false;

    BukoDispenser.stop();
    SugarTimer.stop();
    Pump.stop();

    Mixer.stop();
    Heater.stop();

    CookingTimer1.stop();
    CookingTimer2.stop();
    CoolingTimer.stop();

    Linear.stop();

    Extruder.stop();
    Conveyor.stop();
    TimerCutterDelay.stop();
    Cutter.stop();
    cutStat = 0;
    DisableSteppers();
}

void RunExtrude()
{
    RunCut();
    Conveyor.relayOn();
    Extruder.relayOn();
}

void RunCut()
{
    switch (cutStat)
    {
    case 0:
        TimerCutterDelay.run();
        if (TimerCutterDelay.isTimerCompleted() == true)
        {
            currentMillis2 = millis();
            previousMillis2 = currentMillis2;
            cutStat = 1;
        }
        break;
    case 1:
        cutterCutRun();
        break;
    default:
        break;
    }
}

void cutterCutRun()
{
    currentMillis2 = millis();
    if (currentMillis2 - previousMillis2 >= interval2)
    {
        // save the last time you blinked the LED
        previousMillis2 = currentMillis2;
        Cutter.relayOff();
        TimerCutterDelay.start();
        cutStat = 0;
    }
    else
    {
        Cutter.relayOn();
    }
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

        case ClickEncoder::DoubleClicked:
            refreshScreen = 1;
            if (settingsFlag)
            {
                if (fastScroll == false)
                {
                    fastScroll = true;
                }
                else
                {
                    fastScroll = false;
                }
            }
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
                    if (fastScroll == true)
                    {
                        parametersTimer[currentSettingScreen] += 1;
                    }
                    else
                    {
                        parametersTimer[currentSettingScreen] += 0.1;
                    }
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
                    if (fastScroll == true)
                    {
                        parametersTimer[currentSettingScreen] -= 1;
                    }
                    else
                    {
                        parametersTimer[currentSettingScreen] -= 0.1;
                    }
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
                if (BukoDispenser.getMotorState() == true)
                {
                    BukoDispenser.relayOff();
                }
                else
                {
                    BukoDispenser.relayOn();
                }
            }
            else if (currentTestMenuScreen == 1)
            {
                if (testSugarDispenser == true)
                {
                    testSugarDispenser = false;
                    DisableSteppers();
                }
                else
                {
                    testSugarDispenser = true;
                    EnableSteppers();
                }
            }
            else if (currentTestMenuScreen == 2)
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
            else if (currentTestMenuScreen == 3)
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
            else if (currentTestMenuScreen == 4)
            {
                if (Heater.getMotorState() == true)
                {
                    Heater.relayOff();
                }
                else
                {
                    Heater.relayOn();
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
                if (testRunCut == true)
                {
                    testRunCut = false;
                }
                else
                {
                    testRunCut = true;
                    TimerCutterDelay.start();
                    cutStat = 0;
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
    lcd.print("Status: " + job);
    lcd.setCursor(0, 2);
    lcd.print("Timer: ");
    lcd.setCursor(7, 2);
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
            if (fastScroll == true)
            {
                lcd.write(1);
            }
            else
            {
                lcd.write(2);
            }
        }
    }
    else if (runAutoFlag == true)
    {
        if (runAutoFlag == true && RunAutoCommand == 2)
        {
            PrintRunAuto("Sugar", SugarTimer.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 1)
        {
            PrintRunAuto("Liquid", Pump.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 3)
        {
            PrintRunAuto("Caramelize", CookingTimer1.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 4)
        {
            PrintRunAuto("Buko", BukoDispenser.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 5)
        {
            PrintRunAuto("Cooking", CookingTimer2.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 6)
        {
            PrintRunAuto("Cooling", CoolingTimer.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 7)
        {
            PrintRunAuto("To Extrude", Linear.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 8)
        {
            switch (cutStat)
            {
            case 0:
                PrintRunAuto("Extruding", TimerCutterDelay.getTimeRemaining());
                break;
            case 1:
                PrintRunAuto("Cutting", "N/A");
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
        refreshScreen = false;
    }
}

void setupJumper()
{
}

void setup()
{
    // Encoder Setup
    encoder = new ClickEncoder(3, 2, 4); // Actual
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
    refreshScreen = true;
    Serial.begin(9600);

     //saveSettings(); // Disable upon Initialiaze
    setDispenser();
    DisableSteppers();
    loadSettings();
    setTimer();
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
            refreshScreen = true;
        }
    }

    if (testMenuFlag == true)
    {
        runAllTest();
    }
}