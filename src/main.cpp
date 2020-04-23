#include <Arduino.h>

// IO Pins
//--------------------------------------------------
#define POT_PIN_1 36
#define OUTPUT_PIN_1 39
#define LED_PIN_1 33

#define POT_PIN_2 36
#define OUTPUT_PIN_2 39
#define LED_PIN_2 33

#define POT_PIN_3 36
#define OUTPUT_PIN_3 39
#define LED_PIN_3 33

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
#define THERMO_DO 34    // Serial Data Out
#define THERMO_CLK 32   // Serial Clock

#define THERMO_CS_1 35  // Slave Select 1
#define THERMO_CS_2 35  // Slave Select 2
#define THERMO_CS_3 35  // Slave Select 3

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

double KP_1 = .12;
double KI_1 = .0003;
double KD_1 = 0;

double KP_2 = .12;
double KI_2 = .0003;
double KD_2 = 0;

double KP_3 = .12;
double KI_3 = .0003;
double KD_3 = 0;

double temperature1, setPoint1, outputVal1;
double temperature2, setPoint2, outputVal2;
double temperature3, setPoint3, outputVal3;

AutoPID PID_1(&temperature1, &setPoint1, &outputVal1, OUTPUT_MIN, OUTPUT_MAX, KP_1, KI_1, KD_1);
AutoPID PID_2(&temperature2, &setPoint2, &outputVal2, OUTPUT_MIN, OUTPUT_MAX, KP_2, KI_2, KD_2);
AutoPID PID_3(&temperature3, &setPoint3, &outputVal3, OUTPUT_MIN, OUTPUT_MAX, KP_3, KI_3, KD_3);

unsigned long lastTempUpdate;

// Call repeatedly in loop, only updates after a certain time interval, returns true if update happened
bool updateTemperature();

// For reading software pot and hardware pot
int potRead();
int previousHardwPotValue = 0;
int previousBlynkPotValue = 0;
int previousPotValue = 0;
int V1_value = 0;
int V2_value = 0;
int V3_value = 0;

void setup()
{
  Blynk.begin(auth, ssid, pass);
  Serial.begin(9600);
  thermocouple.begin(THERMO_CLK, THERMO_CS_1, THERMO_DO);
  thermocouple.begin(THERMO_CLK, THERMO_CS_2, THERMO_DO);
  thermocouple.begin(THERMO_CLK, THERMO_CS_3, THERMO_DO);

  lcd.begin(16, 2);
  lcd.createChar(0, degree);

  pinMode(POT_PIN_1, INPUT); pinMode(OUTPUT_PIN_1, OUTPUT); pinMode(LED_PIN_1, OUTPUT);
  pinMode(POT_PIN_2, INPUT); pinMode(OUTPUT_PIN_2, OUTPUT); pinMode(LED_PIN_2, OUTPUT);
  pinMode(POT_PIN_3, INPUT); pinMode(OUTPUT_PIN_3, OUTPUT); pinMode(LED_PIN_3, OUTPUT);

  ledcAttachPin(OUTPUT_PIN_1, 0); ledcSetup(0, 5000, 8);
  ledcAttachPin(OUTPUT_PIN_2, 1); ledcSetup(1, 5000, 8);
  ledcAttachPin(OUTPUT_PIN_3, 2); ledcSetup(2, 5000, 8);

  delay(500);

  //if temperature is more than 4 degrees below or above setpoint, OUTPUT will be set to min or max respectively
  PID_1.setBangBang(4); PID_2.setBangBang(4); PID_3.setBangBang(4);
  //set PID update interval to 4000ms
  PID_1.setTimeStep(4000); PID_2.setTimeStep(4000); PID_3.setTimeStep(4000);
}

/*--------------------------------------------------------------------------------------------------------------------*/
void loop()
{
  Blynk.run();

  lcd.clear();
  lcd.setCursor(0, 0);

  updateTemperature();
  setPoint1 = potRead();
  PID_1.run();
  ledcWrite(0, outputVal);
  digitalWrite(LED_PIN_1, PID_1.atSetPoint(1)); //light up LED when we're at setpoint +-1 degree

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