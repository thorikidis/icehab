/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:
    https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "TMPL4vLs-XKTJ"
#define BLYNK_TEMPLATE_NAME "IceHab"

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
#define USE_ESP32_DEV_MODULE
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI

#include "BlynkEdgent.h"


//void setup()
//{
//  Serial.begin(115200);
//  delay(100);
//
//  BlynkEdgent.begin();
//}
//
//void loop() {
//  BlynkEdgent.run();
//}

// *************************************************************/
//#define BLYNK_TEMPLATE_ID "TMPL4vLs-XKTJ"
//#define BLYNK_TEMPLATE_NAME "Temperatur"
//#define BLYNK_AUTH_TOKEN "denu6kjNliVJyZ2-qGbKvRlhl5QR7yoe"
///////////

///////////Libraries
#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>
//#include <BlynkSimpleEsp32.h>
#include "time.h"
#include "FastLED.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>


Preferences preferences;
const char* ntpServer = "pool.ntp.org";


const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

// Global variables to store Blynk times
int startHour;
int startMinute;
int startSecond;
int stopHour;
int stopMinute;
int stopSecond;
bool selectedDays[7];
unsigned long tzOffset;  //timeZoneOffset

float waterTemp;
float airTemp;

#define DHTPIN 4       // DHT22 sensor data pin
#define DHTTYPE DHT22  // DHT22 sensor model

DHT dht(DHTPIN, DHTTYPE);

///Pins
int l1 = 13;  //12;  // Output connected to D12
int l2 = 12;  //13;  // Output connected to D13
int l3 = 14;  //14;  // Output connected to D14
int l4 = 25;
int l5 = 26;
int l6 = 27;
int l7 = 16;
int l8 = 15;
int l9 = 33;
int waterSwitch = 32;
const int oneWireBus = 5; //the S1 sensor defined on pin5

//////////C1
int setpointV1 = 25;  // Adjust this setpoint value as needed
int setpointV2 = 2;
int mainSetpointV3 = 0;
int systemState = 0;  // 0: Idle, 1: Demand

// Time variables (in milliseconds)
unsigned long timeV4 = 8000;  // Time to wait in Idle state
unsigned long timeV6 = 5000;  // Duration of Period A
unsigned long timeV7 = 5000;  // Duration of Period B
unsigned long timeV8 = 5000;  // Duration of Period C
float tempCalibV5 = 0.0;

unsigned long startTime;  // Variable to store the start time
bool condition_for_timeV4 = true;
bool coolingSystem = false;
bool turnOff = false;
bool timeCondition = false;
unsigned long idleStateEnterTime = 0;
bool delayTime = false;
int mainButton = 0;
bool mainTurnOff = false;

////////c2
bool cleaningSystem = false;
bool manualCleaning = false;
bool autoCleaning = true;
bool L5 = false;
bool L6 = false;

unsigned long manualCleaningStartTime = 0;
bool manualCleaningStartCondition = false;
unsigned long manualCleaningStopTime = 0;
bool c3andc4condition = false;
bool turnOffC2 = false;

///////////C3
int a9 = 3;
int a11 = 2;
unsigned long a13 = 5000;
float a14_TempCalib = 0.0;
unsigned long lastActivationTime = 0;
unsigned long onTime = 0;
unsigned long offTime = 0;
bool schedule = false;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

//////////C4
bool c4 = false;
unsigned long sensorHighTime = 0;
bool outputActivated = false;
unsigned long outputDeactivationTime = 0;

//////////C5
#define NUM_LEDS1 30
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
CRGB leds1[NUM_LEDS1];
#define PIN1 18
int data = 255;
int r, g, b;



///Prefernce
bool preferencesCondition = true;
bool sendingOldData = true;
////////LCD variables
unsigned long previousMillisLcd = 0;
int displayStateLcd = 0;                                // 0: Wifi Connected, 1: TemperatureS1, 2: TemperatureS2
const unsigned long displayIntervalLcd = 2000;          // Display interval in milliseconds
const unsigned long temperatureDisplayDuration = 3000;  // Duration to display each temperature in milliseconds

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Galaxy A24 C1E6";
char pass[] = "pizzafries";

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void myTimerEvent() {
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  // TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  //  sensors.requestTemperatures();
  //  float watertemperature = sensors.getTempCByIndex(0);
  //  temp = dht.readTemperature();
  //hum = dht.readHumidity();
  char tempStr[10];  // Assuming a maximum of 10 characters for the string

  // Convert float to string with 2 decimal places
  dtostrf(waterTemp, 5, 1, tempStr);  // 6 is the width, including the decimal point and 2 decimal places

  // Now tempStr contains the string representation of waterTemp with 2 decimal places

  // Convert the string back to float
  if (sendingOldData == false) {
    float waterTempFloat = atof(tempStr);
    Blynk.virtualWrite(V0, waterTempFloat);
    Blynk.virtualWrite(V1, airTemp);


    //  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
    //  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
    Serial.println("---");
  } else {
    Blynk.virtualWrite(V3, setpointV1);
    Blynk.virtualWrite(V9, coolingSystem);
    Blynk.virtualWrite(V11, L5);
    Blynk.virtualWrite(V12, L6);
    Blynk.virtualWrite(V17, c4);
    if (turnOffC2 == false) {
      Blynk.virtualWrite(V13, 2);
    } else if (manualCleaning == true && autoCleaning == false) {
      Blynk.virtualWrite(V13, 1);
    } else if (manualCleaning == false && autoCleaning == true) {
      Blynk.virtualWrite(V13, 0);
    }
    delay(1000);
    Blynk.virtualWrite(V2, setpointV2);
    Blynk.virtualWrite(V4, timeV4 / 1000);
    Blynk.virtualWrite(V5, tempCalibV5);
    Blynk.virtualWrite(V6, timeV6 / 1000);
    Blynk.virtualWrite(V7, timeV7 / 1000);
    Blynk.virtualWrite(V8, timeV8 / 1000);
    Blynk.virtualWrite(V14, a9);
    Blynk.virtualWrite(V15, a11);
    Blynk.virtualWrite(V16, a13 / 1000);
    Blynk.virtualWrite(V19, a14_TempCalib);
    sendingOldData = false;
  }
}

void startPeriodA() {
  // Code for actions during Period A
  digitalWrite(l1, HIGH);
  digitalWrite(l2, HIGH);
  digitalWrite(l3, LOW);
  Serial.println("Period A");
}

void startPeriodB() {
  // Code for actions during Period B
  digitalWrite(l1, HIGH);
  digitalWrite(l2, LOW);
  digitalWrite(l3, HIGH);
  Serial.println("Period B");
}

void startPeriodC() {
  // Code for actions during Period C
  digitalWrite(l1, LOW);
  digitalWrite(l2, LOW);
  digitalWrite(l3, LOW);
  Serial.println("Period C");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please Connect");
  lcd.setCursor(0, 1);
  lcd.print("From Blynk App");
  pinMode(l1, OUTPUT);
  pinMode(l2, OUTPUT);
  pinMode(l3, OUTPUT);
  pinMode(l4, OUTPUT);
  pinMode(l5, OUTPUT);
  pinMode(l6, OUTPUT);
  pinMode(l7, OUTPUT);
  pinMode(l8, OUTPUT);
  pinMode(l9, OUTPUT);
  pinMode(waterSwitch, INPUT_PULLUP);
  FastLED.addLeds<LED_TYPE, PIN1, COLOR_ORDER>(leds1, NUM_LEDS1).setCorrection(TypicalLEDStrip);
  preferences.begin("my-app", false);
  dht.begin();
  sensors.begin();
  pinMode(2, OUTPUT);


  Serial.println("DHT22 test");
  BlynkEdgent.begin();
  //  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
  //  configTime(5 * 3600, 0, "pool.ntp.org"); //For Pakistan
  configTime(3 * 3600, 0, "pool.ntp.org");  //For Greece
  //  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(500);
}

BLYNK_WRITE(V3)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  setpointV1 = param.asInt();
  preferences.putInt("setpointV1", setpointV1);
}


BLYNK_WRITE(V2)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  setpointV2 = param.asInt();
  preferences.putInt("setpointV2", setpointV2);
}

BLYNK_WRITE(V4)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  timeV4 = param.asInt() * 1000;
  preferences.putULong("timeV4", timeV4);
}

BLYNK_WRITE(V5)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asFloat());
  tempCalibV5 = param.asFloat();
  preferences.putFloat("tempCalibV5", tempCalibV5);
}

BLYNK_WRITE(V6)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  //delay(2000);
  timeV6 = param.asInt() * 1000;
  preferences.putULong("timeV6", timeV6);
}

BLYNK_WRITE(V7)  // Executes when the value of virtual pin 0 changesv9
{
  Serial.println(param.asInt());
  timeV7 = param.asInt() * 1000;
  preferences.putULong("timeV7", timeV7);
}

BLYNK_WRITE(V8)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  timeV8 = param.asInt() * 1000;
  preferences.putULong("timeV8", timeV8);
}

BLYNK_WRITE(V9)  // Executes when the value of virtual pin 0 changes
{

  Serial.println(param.asInt());
  if (param.asInt() == 1) {
    coolingSystem = true;
    preferences.putBool("coolingSystem", coolingSystem);
  } else {
    coolingSystem = false;
    preferences.putBool("coolingSystem", coolingSystem);
  }
}


BLYNK_WRITE(V10)  // Executes when the value of virtual pin 0 changes
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime()) {
    Serial.println(String("Start: ") + t.getStartHour() + ":" + t.getStartMinute() + ":" + t.getStartSecond());

    startHour = t.getStartHour();
    startMinute = t.getStartMinute();
    startSecond = t.getStartSecond();
    preferences.putInt("startHour", startHour);
    preferences.putInt("startMinute", startMinute);
    preferences.putInt("startSecond", startSecond);
  } else if (t.isStartSunrise()) {
    Serial.println("Start at sunrise");
  } else if (t.isStartSunset()) {
    Serial.println("Start at sunset");
  } else {
    // Do nothing
  }

  // Process stop time

  if (t.hasStopTime()) {
    Serial.println(String("Stop: ") + t.getStopHour() + ":" + t.getStopMinute() + ":" + t.getStopSecond());
    stopHour = t.getStopHour();
    stopMinute = t.getStopMinute();
    stopSecond = t.getStopSecond();
    preferences.putInt("stopHour", stopHour);
    preferences.putInt("stopMinute", stopMinute);
    preferences.putInt("stopSecond", stopSecond);
  }

  else if (t.isStopSunrise()) {
    Serial.println("Stop at sunrise");
  } else if (t.isStopSunset()) {
    Serial.println("Stop at sunset");
  } else {
    // Do nothing: no stop time was set
  }

  // Process timezone
  // Timezone is already added to start/stop time
  Serial.print("Time Zone");
  Serial.println(String("Time zone: ") + t.getTZ());

  // Get timezone offset (in seconds)
  Serial.print("Time Zone Offset");
  Serial.println(String("Time zone offset: ") + t.getTZ_Offset());

  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      Serial.println(String("Day ") + i + " is selected");
    }
  }

  for (int i = 1; i <= 6; i++) {
    selectedDays[i] = t.isWeekdaySelected(i);
    Serial.print("day");
    Serial.print(i);
    Serial.print("   ");
    Serial.println(selectedDays[i]);
  }
  selectedDays[0] = t.isWeekdaySelected(7);
  preferences.putBytes("selectedDays", (uint8_t*)selectedDays, sizeof(selectedDays));

  Serial.println();
  cleaningSystem = false;
  schedule = false;
  tzOffset = t.getTZ_Offset();
  preferences.putULong("tzOffset", tzOffset);
  configTime(t.getTZ_Offset(), 0, "pool.ntp.org");
  //delay(3000);
}


BLYNK_WRITE(V11)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  if (param.asInt() == 1) {
    L5 = true;
    preferences.putBool("L5", L5);
  } else {
    L5 = false;
    preferences.putBool("L5", L5);
  }
}


BLYNK_WRITE(V12)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  if (param.asInt() == 1) {
    L6 = true;
    preferences.putBool("L6", L6);
  } else {
    L6 = false;
    preferences.putBool("L6", L6);
  }
}


BLYNK_WRITE(V13)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  //delay(3000);
  if (param.asInt() == 1) {
    manualCleaning = true;
    autoCleaning = false;
    turnOffC2 = true;
    preferences.putBool("manualCleaning", manualCleaning);
    preferences.putBool("autoCleaning", autoCleaning);
    preferences.putBool("turnOffC2", turnOffC2);
    digitalWrite(l4, HIGH);
  } else if (param.asInt() == 0) {
    manualCleaning = false;
    autoCleaning = true;
    turnOffC2 = true;
    preferences.putBool("manualCleaning", manualCleaning);
    preferences.putBool("autoCleaning", autoCleaning);
    preferences.putBool("turnOffC2", turnOffC2);
    if (!c3andc4condition) {
      //digitalWrite(l4, LOW);
    }
  } else if ((param.asInt() == 2)) {
    turnOffC2 = false;
    preferences.putBool("turnOffC2", turnOffC2);
  }
}


BLYNK_WRITE(V14)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  a9 = param.asInt();
  preferences.putInt("a9", a9);
}

BLYNK_WRITE(V15)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  a11 = param.asInt();
  preferences.putInt("a11", a11);
}

BLYNK_WRITE(V16)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  a13 = param.asInt() * 1000;
  preferences.putULong("a13", a13);
}


BLYNK_WRITE(V17)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asInt());
  if (param.asInt() == 1) {
    c4 = true;
    preferences.putBool("c4", c4);

  } else {
    c4 = false;
    preferences.putBool("c4", c4);
    //digitalWrite(l9, LOW);
  }
}

BLYNK_WRITE(V19)  // Executes when the value of virtual pin 0 changes
{
  Serial.println(param.asFloat());
  a14_TempCalib = param.asFloat();
  preferences.putFloat("a14_TempCalib", a14_TempCalib);
}



//BLYNK_WRITE(V20)
//{
//data = param.asInt();
//static1(r, g, b,data);
//}

BLYNK_WRITE(V18) {
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
  static1(r, g, b, data);
  preferences.putInt("r", r);
  preferences.putInt("g", g);
  preferences.putInt("b", b);
}

void static1(int r, int g, int b, int brightness) {
  FastLED.setBrightness(brightness);
  for (int i = 0; i < NUM_LEDS1; i++) {
    leds1[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

void preferencesh() {
  setpointV1 = preferences.getInt("setpointV1", setpointV1);               //V3
  coolingSystem = preferences.getBool("coolingSystem", coolingSystem);     //V9
  manualCleaning = preferences.getBool("manualCleaning", manualCleaning);  //V13
  autoCleaning = preferences.getBool("autoCleaning", autoCleaning);
  turnOffC2 = preferences.getBool("turnOffC2", turnOffC2);
  L5 = preferences.getBool("L5", L5);  //V11
  L6 = preferences.getBool("L6", L6);  //V12
  c4 = preferences.getBool("c4", c4);  //V17
  //Schedule
  startHour = preferences.getInt("startHour", startHour);
  startMinute = preferences.getInt("startMinute", startMinute);
  startSecond = preferences.getInt("startSecond", startSecond);
  stopHour = preferences.getInt("stopHour", stopHour);
  stopMinute = preferences.getInt("stopMinute", stopMinute);
  stopSecond = preferences.getInt("stopSecond", stopSecond);
  tzOffset = preferences.getULong("tzOffset", tzOffset);
  configTime(tzOffset, 0, "pool.ntp.org");
  Serial.println("tzOffset  ");
  Serial.print(tzOffset);

  ///Admin
  setpointV2 = preferences.getInt("setpointV2", setpointV2);             //V2
  timeV4 = preferences.getULong("timeV4", timeV4);                       //V4
  tempCalibV5 = preferences.getFloat("tempCalibV5", tempCalibV5);        //V5
  timeV6 = preferences.getULong("timeV6", timeV6);                       //V6
  timeV7 = preferences.getULong("timeV7", timeV7);                       //V7
  timeV8 = preferences.getULong("timeV8", timeV8);                       //V8
  a9 = preferences.getInt("a9", a9);                                     //V14
  a11 = preferences.getInt("a11", a11);                                  //V15
  a13 = preferences.getULong("a13", a13);                                //V16
  a14_TempCalib = preferences.getFloat("a14_TempCalib", a14_TempCalib);  //V19


  r = preferences.getInt("r", r);
  g = preferences.getInt("g", g);
  b = preferences.getInt("b", b);
  static1(r, g, b, data);

  preferences.getBytes("selectedDays", (uint8_t *)selectedDays, sizeof(selectedDays));
}

void loop() {
  if (!Blynk.connected() || WiFi.status() != WL_CONNECTED) {
    sendingOldData = true;
  }

  if (preferencesCondition == true) {
    Serial.println("Reading Preferences.....");
    delay(1000);
    Serial.println("Getting Old Readings");
    preferencesh();
    delay(3000);
    preferencesCondition = false;
  }

  mainSetpointV3 = setpointV1 + setpointV2;  ////mainSetpointV3 this is main setPoint of C1 Temperature
  sensors.requestTemperatures();
  float watertemperatureC = sensors.getTempCByIndex(0);
  // float temperatureS1 = 15.02 + tempCalibV5;
  float temperatureS1 = watertemperatureC + tempCalibV5;
  waterTemp = temperatureS1;
  Serial.print("Temperature S1: ");
  Serial.print(temperatureS1);
  Serial.println("ºC");
  // float temperatureS2 = 36 + a14_TempCalib;

  float temperatureS2 = dht.readTemperature() + a14_TempCalib;
  airTemp = temperatureS2;
  ///C3 Calculation
  float a10 = a9 + temperatureS1;
  float a12 = a10 + a11;
  float mainSetpointforC3 = a12;  ////mainSetpointforC3 this is main setPoint of C3 Water Temperature


  Serial.print("Temperature S2: ");
  Serial.println(temperatureS2);
  Serial.print("mainSetpointC1: ");
  Serial.println(mainSetpointV3);
  Serial.print("mainSetpointC3: ");
  Serial.println(mainSetpointforC3);

  Serial.print("timeV4: ");
  Serial.println(timeV4 / 1000);
  //  Serial.print("timeV5:  ");
  //  Serial.println(timeV5);
  Serial.print("timeV6:  ");
  Serial.println(timeV6 / 1000);
  Serial.print("timeV7:  ");
  Serial.println(timeV7 / 1000);
  Serial.print("timeV8: ");
  Serial.println(timeV8 / 1000);

  // && coolingSystem == true

  ///C1 COde
  if (!isnan(temperatureS1) && temperatureS1 > mainSetpointV3 && coolingSystem == true) {
    timeCondition = true;
    mainTurnOff = true;
    if (systemState == 0) {
      // Transition to Demand state
      turnOff = false;
      systemState = 1;
      startTime = millis();  // Record the start time
      Serial.println("Demand State");
    }
  } else {
    // Transition to Idle state
    //    if (timeCondition == true) {
    //      startTime = millis();
    //      timeCondition = false;
    //    }
    //    systemState = 0;

    //    turnOff = true;
    if (systemState != 0 || mainTurnOff == true) {
      Serial.println("Idle State");
      idleStateEnterTime = millis();  // Record the time when entering the Idle state
      systemState = 0;                // Update system state to Idle
      delayTime = true;
      mainTurnOff = false;
    } else {
      // Check if 2 seconds have passed since entering the Idle state
      if (millis() - idleStateEnterTime >= timeV4) {
        Serial.println("2 seconds have passed in Idle State");
        startPeriodC();
        condition_for_timeV4 = true;
        delayTime = false;
        // Additional actions after 2 seconds in Idle State can be added here
      }
    }
  }

  // Control outputs based on system state and elapsed time
  if ((systemState == 1 && condition_for_timeV4 == true) || turnOff == true) {
    if (turnOff == true) {
      Serial.println(startTime);
    }
    unsigned long elapsedTime = millis() - startTime;

    if (elapsedTime < timeV4) {
      // Wait for timeV4
      if (condition_for_timeV4 == true) {
        digitalWrite(l1, LOW);
        digitalWrite(l2, LOW);
        digitalWrite(l3, LOW);
        Serial.println("Waiting for timeV4");
      }

    } else {
      if (turnOff == false) {
        condition_for_timeV4 = false;
        startTime = millis();
      }
    }
  }
  if ((systemState == 1 && condition_for_timeV4 == false) || (delayTime == true && condition_for_timeV4 == false)) {
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime < (timeV6)) {
      // Period A
      startPeriodA();
    } else if (elapsedTime < (timeV6 + timeV7)) {
      // Period B
      startPeriodB();
    } else if (elapsedTime < (timeV6 + timeV7 + timeV8)) {
      // Period C
      startPeriodC();
    } else {
      // Reset to idle state after completing Period C
      systemState = 0;
      Serial.println("Resetting to New Time");
      if (delayTime == true) {
        startTime = millis();
      }
    }
  }
  //}

  // else {
  //    // Idle state
  //    digitalWrite(l1, LOW);
  //    digitalWrite(l2, LOW);
  //    digitalWrite(l3, LOW);
  //  }

  //  delay(1000);  // Adjust the delay as needed


  ///Blynk Start
  BlynkEdgent.run();
  //  Blynk.run();
  timer.run();  // Initiates BlynkTimer.
  printLocalTime();


  ////C2 Code
  // Get and compare times
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  // Compare with Blynk start time
  // Compare with Blynk start time
  //  Serial.print("Start Hours");
  //  Serial.println(startHour);
  Serial.print("days  ");
  Serial.println(timeinfo.tm_wday);
  if (selectedDays[timeinfo.tm_wday]) {
    if ((timeinfo.tm_hour > startHour && schedule == false) || (timeinfo.tm_hour == startHour && timeinfo.tm_min >= startMinute && schedule == false)) {
      // Perform action for Blynk start time
      Serial.println("Blynk start time reached!");
      cleaningSystem = true;
    }

    // Compare with Blynk stop time

    if (timeinfo.tm_hour == stopHour && timeinfo.tm_min == stopMinute) {
      // Perform action for Blynk stop time
      schedule = true;
      Serial.println("Blynk stop time reached!");
      digitalWrite(2, LOW);
      cleaningSystem = false;
    }
  }
  Serial.print("schedule   ");
  Serial.println(schedule);
  Serial.print("Auto Cleaning   ");
  Serial.println(cleaningSystem);
  //  Serial.print("TEst  ");
  //  Serial.println(timeinfo.tm_wday);

  //digitalWrite(2, HIGH);
  if (autoCleaning == true && manualCleaning == false && c3andc4condition == false && turnOffC2 == true) {
    cleaning();
  }
  if ((manualCleaning == true || c3andc4condition == true) && turnOffC2 == true) {
    Serial.println("Manual Cleaning ");
    digitalWrite(l4, HIGH);
    if (L5 == true) {
      digitalWrite(l5, HIGH);
    } else {
      digitalWrite(l5, LOW);
    }

    if (L6 == true) {
      digitalWrite(l6, HIGH);
    } else {
      digitalWrite(l6, LOW);
    }
  }
  if (turnOffC2 == false) {
    Serial.println("C2 is Off");
    digitalWrite(l4, LOW);
    digitalWrite(l5, LOW);
    digitalWrite(l6, LOW);
  }

  if (temperatureS2 < 1) {
    if (!manualCleaningStartCondition) {

      manualCleaningStartTime = millis();
      manualCleaningStartCondition = true;
    }
    manualCleaningStopTime = millis();  // Reset the deactivation timer
  } else {
    Serial.print("DETIEM");
    Serial.print(manualCleaningStopTime);
    manualCleaningStartCondition = false;
    // If the output was activated, turn it off after the delay
    if (millis() - manualCleaningStopTime >= a13) {
      manualCleaningStartCondition = false;
      c3andc4condition = false;
    }
  }

  // Check if the delay has passed and activate the output
  if (manualCleaningStartCondition == true && (millis() - manualCleaningStartTime >= a13)) {
    digitalWrite(l4, HIGH);
    c3andc4condition = true;
    manualCleaningStartCondition = false;  // Reset the flag
  }

  ///////////C3 CODE
  if (temperatureS2 > mainSetpointforC3) {
    Serial.println("In C3");
    offTime = millis();
    if (millis() - onTime >= a13) {
      // If 2 seconds have passed since condition was last met, print something
      turnOnOutputs();
    }
  } else {
    lastActivationTime = 0;  // Reset the time if condition is not met
    if (millis() - offTime >= a13) {
      // If 2 seconds have passed since condition was last met, print something
      turnOffOutputs();
    }
    onTime = millis();
  }


  //////////C4 CODE
  Serial.print("c4    ");
  Serial.println(c4);
  int sensorValue = digitalRead(waterSwitch);
  //temperatureS2 > mainSetpointforC3 //sensorValue == LOW && c4 == true
  if (sensorValue == LOW && c4 == true) {
    // Sensor is triggered
    //digitalWrite(2, HIGH);
    if (!outputActivated) {
      // If the output is not already activated, start the delay
      sensorHighTime = millis();
      outputActivated = true;
    }
    outputDeactivationTime = millis();  // Reset the deactivation timer
  } else {
    // Sensor is not triggered
    // digitalWrite(2, LOW);
    // if (outputActivated) {
    outputActivated = false;
    Serial.print("C4 WaterLevel Off");
    Serial.print(outputDeactivationTime);
    // If the output was activated, turn it off after the delay
    if (millis() - outputDeactivationTime >= 10000) {
      outputActivated = false;
      activateOutput();

      //}
    }
  }

  // Check if the delay has passed and activate the output
  if (outputActivated == true && (millis() - sensorHighTime >= 10000)) {
    deactivateOutput();
    outputActivated = false;  // Reset the flag
  }

  ////////////LCD
  // if (WiFi.status() == WL_CONNECTED) {
  //   lcd.setCursor(0, 0);
  //   lcd.print("Wifi Connected");
  //   lcd.setCursor(0, 1);
  //   lcd.print("Press For 10Sec");
  //   //delay(1000);
  // }
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis1 = millis();
    if (currentMillis1 - previousMillisLcd >= displayIntervalLcd) {
      previousMillisLcd = currentMillis1;
      if (displayStateLcd == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wifi Connected");
        lcd.setCursor(0, 1);
        lcd.print("Reset For 10Sec");
        displayStateLcd = 1;
      } else if (displayStateLcd == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp S1: ");
        lcd.print(temperatureS1);
        lcd.setCursor(0, 1);
        lcd.print("Temp S2: ");
        lcd.print(temperatureS2);
        displayStateLcd = 0;
      } else if (displayStateLcd == 2) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Temp S2: ");
        lcd.print(temperatureS2);
        displayStateLcd = 1;
      }
    }
  }
  Serial.println();
  preferencesh();
}




void activateOutput() {
  digitalWrite(l9, HIGH);  // Activate the output connected to pin 24
  Serial.println("Output activated!");
  // Additional actions can be added here if needed
}

void deactivateOutput() {
  digitalWrite(l9, LOW);  // Deactivate the output connected to pin 24
  Serial.println("Output deactivated!");
  // Additional actions can be added here if needed
}

void cleaning() {
  if (cleaningSystem == true) {
    digitalWrite(l4, HIGH);
    //digitalWrite(2, HIGH);
  }
  if (cleaningSystem == false) {
    digitalWrite(l4, LOW);
  }
  if (L5 == true && cleaningSystem == true) {
    digitalWrite(l5, HIGH);
  } else {
    digitalWrite(l5, LOW);
  }
  if (L6 == true && cleaningSystem == true) {
    digitalWrite(l6, HIGH);
  } else {
    digitalWrite(l6, LOW);
  }
}

void turnOnOutputs() {
  digitalWrite(l7, HIGH);
  digitalWrite(l8, HIGH);
}

void turnOffOutputs() {
  digitalWrite(l7, LOW);
  digitalWrite(l8, LOW);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
}
