#include <Arduino.h>

//--------------------------------------------------
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = "YourAuthToken";
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";
//--------------------------------------------------

#include <AutoPID.h>

//pins
#define POT_PIN 36
#define OUTPUT_PIN 39
#define TEMP_PROBE_PIN 34
#define LED_PIN 35
#define TEMP_READ_DELAY 800

//pid settings and gains
double OUTPUT_MIN = 0;
double OUTPUT_MAX = 255;
double KP = .12;
double KI = .0003;
double KD = 0;

double temperature, setPoint, outputVal;

//input/output variables passed by reference, so they are updated automatically
AutoPID myPID(&temperature, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);
unsigned long lastTempUpdate; //tracks clock time of last temp update

//call repeatedly in loop, only updates after a certain time interval, returns true if update happened
bool updateTemperature();

int potRead();
int previousHardwPotValue = 0;
int previousBlynkPotValue = 0;
int previousPotValue = 0;
int V1_value = 0;

void setup()
{
  Blynk.begin(auth, ssid, pass);

  pinMode(POT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  ledcAttachPin(TEMP_PROBE_PIN, 0);
  ledcSetup(0, 5000, 8);

  //if temperature is more than 4 degrees below or above setpoint, OUTPUT will be set to min or max respectively
  myPID.setBangBang(4);
  //set PID update interval to 4000ms
  myPID.setTimeStep(4000);
}

void loop()
{
  Blynk.run();
  updateTemperature();
  //setPoint = analogRead(POT_PIN);
  setPoint = potRead();
  myPID.run(); //call every loop, updates automatically at certain time interval
  ledcWrite(0, outputVal);
  digitalWrite(LED_PIN, myPID.atSetPoint(1)); //light up LED when we're at setpoint +-1 degree
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
    previousPotValue = previousHardwPotValue;
    return previousHardwPotValue;
  }
  else if (V1_value >= 20 + previousBlynkPotValue || V1_value <= previousBlynkPotValue - 20)
  {
    previousBlynkPotValue = V1_value;
    previousPotValue = previousBlynkPotValue;
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
    temperature = analogRead(TEMP_PROBE_PIN); //get temp reading
    lastTempUpdate = millis();
    return true;
  }
  return false;
}