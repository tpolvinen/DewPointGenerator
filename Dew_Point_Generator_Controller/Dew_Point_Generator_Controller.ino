#include <RTClib.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#define voltsIn A0

Adafruit_MCP4725 dac;
RTC_DS3231 rtc;

struct tcontrol
{
  uint8_t duration;
  uint16_t celcius;
}; 
typedef struct tcontrol Tcontrol;

//-------------------- BEGIN MODIFY THIS --------------------

#define DURATION_TEMPS 3  // define how many duration-temperature pairs you want to use

Tcontrol tcontrols[DURATION_TEMPS] = {
  { 10, 0 },   // define particular duration in minutes and temperature in celcius
  { 10, 25 },  // same
  { 10, 50 },  // same -and by the way, you can add more than you defined above, but they won't be used :)
};

//-------------------- END MODIFY THIS ----------------------

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long durationTime = 0;

uint8_t x = 0;
uint16_t temperature;

Tcontrol tcontrol;

// LED blinker variables:
const int ledPin =  LED_BUILTIN;
int ledState = LOW;
const long ledInterval = 750;
unsigned long ledPreviousMillis = 0;


void setup(void) 
{
  Serial.begin(9600);
  dac.begin(0x62); // The I2C Address of the MCP4725
  pinMode(ledPin, OUTPUT);
  
  delay(3000); // wait for console opening


//--------------------BEGIN Real Time Clock setup--------------------
  if (! rtc.begin()) {
    Serial.println();
    Serial.println("Couldn't find RTC");
    Serial.println();
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println();
    Serial.println("RTC lost power, lets set the time!");
    Serial.println();
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
//--------------------END Real Time Clock setup--------------------

  
  tcontrol = tcontrols[0];
  temperature = tcontrol.celcius;
  durationTime = (unsigned long)tcontrol.duration*60*1000; // multiplies minutes in duration to milliseconds
//  durationTime = 3000; // Shortening durationTime for testing
  currentMillis = millis();
  previousMillis = currentMillis;

  printDateAndSettings();
  
  setTemperature(temperature);
}



void loop(void)
{
  currentMillis = millis();
  
  if(currentMillis - previousMillis > durationTime)  // if set duration has passed, set next temperature and duration
  {
    
    x = x+1;
    if(x >= DURATION_TEMPS) x=0;
    tcontrol = tcontrols[x];
    temperature = tcontrol.celcius;
    durationTime = (unsigned long)tcontrol.duration*60*1000;
//    durationTime = 3000; // Shortening durationTime for testing
    previousMillis = currentMillis;
    
    printDateAndSettings();
    
    setTemperature(temperature);
  }
  
  if (currentMillis - ledPreviousMillis >= ledInterval)
  {
    ledPreviousMillis = currentMillis;
    
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    
    digitalWrite(ledPin, ledState);
  }
  
}



void printDateAndSettings()
{
  DateTime now = rtc.now();
  
  Serial.println();
  Serial.print("Started at:\t");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  Serial.print("Settings:\t");
  Serial.print("Duration in min: ");
  Serial.print(tcontrol.duration);
  
}



void setTemperature(uint16_t temperature) 
{
  uint16_t dac_value;
  int adcValueRead = 0;
  float voltageRead = 0;
    
  float dac_expected_output;

  dac_value = temperature * 100 * 0.819; //temperature * 100 => millivolts needed * 0.819 => dac_value needed, in "steps" between 0 and 4096 (resolution), also 4095/5000 = 0.819
  
  dac_expected_output = (5.0/4096.0) * dac_value * 10; // calculates the expected voltage by dividing the max. voltage by the resolution of 4096 in order to get one "step" of resolution, then multiplying it with the number of steps
  dac.setVoltage(dac_value, false);
  delay(250);
  adcValueRead = analogRead(voltsIn);
  voltageRead = (adcValueRead * 5.0 )/ 1024.0;  // gets analog reading (analogread results in an integer between 0 and 1024 (resolution), so multiply it by the max. voltage and divide it by the resolution) 
  
  Serial.print("\tDAC Value: ");
  Serial.print(dac_value);
  
  Serial.print("\tExpected temperature: ");
  Serial.print(dac_expected_output,0);
  Serial.print(" ");
  
  Serial.print("\tVoltage: ");      
  Serial.println(voltageRead,3);      
}
