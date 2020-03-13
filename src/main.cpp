#include <Arduino.h>

// IO Pins
//--------------------------------------------------
#define POT_PIN 36
#define OUTPUT_PIN 39
#define LED_PIN 33
#define TEMP_READ_DELAY 800
//--------------------------------------------------

// Blynk Configurations
//--------------------------------------------------
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = "YourAuthToken";
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";
//--------------------------------------------------

// MAX6675 and LCD Configurations
//--------------------------------------------------
#include <max6675.h>
#define THERMO_DO 34
#define THERMO_CS 35
#define THERMO_CLK 32
MAX6675 thermocouple;

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
uint8_t degree[8] = {140, 146, 146, 140, 128, 128, 128, 128};
#include <Wire.h>
#include <SPI.h>
//--------------------------------------------------

// AutoPID Configuartions
//--------------------------------------------------
#include <AutoPID.h>
double OUTPUT_MIN = 0;
double OUTPUT_MAX = 255;
double KP = .12;
double KI = .0003;
double KD = 0;

double temperature, setPoint, outputVal;
AutoPID myPID(&temperature, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);
unsigned long lastTempUpdate;

// Call repeatedly in loop, only updates after a certain time interval, returns true if update happened
bool updateTemperature();

// For reading software pot and hardware pot
int potRead();
int previousHardwPotValue = 0;
int previousBlynkPotValue = 0;
int previousPotValue = 0;
int V1_value = 0;

void setup()
{
  Blynk.begin(auth, ssid, pass);
  Serial.begin(9600);
  thermocouple.begin(THERMO_CLK, THERMO_CS, THERMO_DO);

  lcd.begin(16, 2);
  lcd.createChar(0, degree);

  pinMode(POT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  ledcAttachPin(OUTPUT_PIN, 0);
  ledcSetup(0, 5000, 8);

  delay(500);

  //if temperature is more than 4 degrees below or above setpoint, OUTPUT will be set to min or max respectively
  myPID.setBangBang(4);
  //set PID update interval to 4000ms
  myPID.setTimeStep(4000);
}

void loop()
{
  Blynk.run();

  lcd.clear();
  lcd.setCursor(0, 0);

  updateTemperature();
  setPoint = potRead();
  myPID.run(); //call every loop, updates automatically at certain time interval
  ledcWrite(0, outputVal);
  digitalWrite(LED_PIN, myPID.atSetPoint(1)); //light up LED when we're at setpoint +-1 degree

  lcd.setCursor(0, 1);
  lcd.print(thermocouple.readCelsius());
}

BLYNK_WRITE(V1)
{
  V1_value = param.asInt();
}

int potRead()
{
  // if the hardware pot is changed, it will return that value, if the Blynk pot is changed, it will return that value
  if (analogRead(POT_PIN) >= 20 + previousHardwPotValue || analogRead(POT_PIN) <= previousHardwPotValue - 20)
  {
    previousHardwPotValue = analogRead(POT_PIN);
    previousPotValue = previousHardwPotValue; // Makes Blynk pot the latest used value
    return previousHardwPotValue;
  }
  else if (V1_value >= 20 + previousBlynkPotValue || V1_value <= previousBlynkPotValue - 20)
  {
    previousBlynkPotValue = V1_value;
    previousPotValue = previousBlynkPotValue; // Makes Hardware pot the latest used value
    return previousBlynkPotValue;
  }
  else
  {
    return previousPotValue;
  }
}

bool updateTemperature()
{
  if ((millis() - lastTempUpdate) > TEMP_READ_DELAY)
  {
    temperature = analogRead(thermocouple.readCelsius());
    lastTempUpdate = millis();
    return true;
  }
  return false;
}