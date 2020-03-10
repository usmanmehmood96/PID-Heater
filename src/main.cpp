#include <Arduino.h>
#include <AutoPID.h>

//#include <OneWire.h>

//pins
#define POT_PIN 35
#define OUTPUT_PIN 36
#define TEMP_PROBE_PIN 32
#define LED_PIN 6

#define TEMP_READ_DELAY 800 //can only read digital temp sensor every ~750ms

//pid settings and gainsx
#define OUTPUT_MIN 0
#define OUTPUT_MAX 255
#define KP .12
#define KI .0003
#define KD 0

double temperature, setPoint, outputVal;

//input/output variables passed by reference, so they are updated automatically
AutoPID myPID(&temperature, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate; //tracks clock time of last temp update

//call repeatedly in loop, only updates after a certain time interval
//returns true if update happened
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
  setPoint = analogRead(POT_PIN);
  myPID.run(); //call every loop, updates automatically at certain time interval
  ledcWrite(0, outputVal);
  digitalWrite(LED_PIN, myPID.atSetPoint(1)); //light up LED when we're at setpoint +-1 degree
}


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "YourAuthToken";
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
}