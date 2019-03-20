#include <Wire.h>
#include <Adafruit_MCP4725.h>
#define voltsIn A0

Adafruit_MCP4725 dac;

struct tcontrol
{
  uint8_t duration;
  uint16_t celcius;
}; 
typedef struct tcontrol Tcontrol;

//-------------------- BEGIN MODIFY THIS --------------------

#define DURATION_TEMPS 3  // define how many duration-temperature pairs you want to use

Tcontrol tcontrols[DURATION_TEMPS] = {
  { 1, 0 },   // define particular duration in minutes and temperature in celcius
  { 1, 25 },  // same
  { 1, 50 },  // same -and by the way, you can add more than you defined above, but they won't be used :)
};

//-------------------- END MODIFY THIS ----------------------

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long durationTime = 0;

uint8_t x = 0;
uint16_t temperature;

Tcontrol tcontrol;



void setup(void) 
{
  Serial.begin(9600);
  dac.begin(0x62); // The I2C Address of the MCP4725
  
  delay(250);
  
  tcontrol = tcontrols[0];
  temperature = tcontrol.celcius;
  durationTime = (unsigned long)tcontrol.duration*60*1000; // multiplies minutes in duration to milliseconds
  currentMillis = millis();
  previousMillis = currentMillis;
  setTemperature(temperature);
}



void loop(void)
{
  currentMillis = millis();
  
  if(currentMillis - previousMillis > durationTime)  // if set duration has passed, set next temperature and duration
  {
    previousMillis = currentMillis;
    x = x+1;
    if(x >= DURATION_TEMPS) x=0;
    tcontrol = tcontrols[x];
    temperature = tcontrol.celcius;
    setTemperature(temperature);
    durationTime = (unsigned long)tcontrol.duration*60*1000;
  }
  
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
  
  Serial.print("DAC Value: ");
  Serial.print(dac_value);
  
  Serial.print("\tExpected temperature: ");
  Serial.print(dac_expected_output,0);
  Serial.print(" ");
  
  Serial.print("\tVoltage: ");      
  Serial.println(voltageRead,3);      
}
