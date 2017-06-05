/***************************************************
  This is a Script to Control a Fog machine with a
  heater, a pump and a PT100 as Temperaturesensor.

  The control Panel has to Switches as Input (Run, Automatic)
  and a Variable Resistor to controll intensity.

  There are two LED´s to inditcate Status. Heat and ready.
 ****************************************************/

#include <Adafruit_MAX31865.h>
#define Loopdelay 1000
#define AutomaticTimeMax 60000UL

/*
State 0: heat_up
State 1:
State 2:
State 3:
State 4:
State 5:
State 6:
state 7:
*/

const int S_HEATUP = 0;
const int S_WAITTILLHOT = 1;
const int S_CHECKINPUTS = 2;
const int S_RUN = 3;
const int S_AUTOMATIC = 4;
const int S_AUTOMATIC_RUN= 5;
const int S_COUNTRYYELLOW = 6;
const int S_WAITCOUNTRYYELLOW = 7;

const int RunSwitchPin = 10;
const int AutomaticSwitchPin = 11;

const int HeatLedPin = 13;
const int ReadyLedPin = 12;
const int HeaterPin = 8;
const int PumpPin = 9;

const int IntensityPin = 0;

unsigned long waitUntil=0;
unsigned long waitAutomatic=0;

const int debug=1;
float FogIntensity ;
unsigned long time;


// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max = Adafruit_MAX31865(5, 4, 3, 2);

// The value of the Rref resistor. Use 430.0!
#define RREF 430.0

void setup() {
  Serial.begin(115200);
  Serial.println("Controller for SLE Cyberfog Fog Machine");

  //Initialize MAX31865 for Temperature reading
  max.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary

  //Initialize all the other Pins
  pinMode(RunSwitchPin, INPUT_PULLUP);
  pinMode(AutomaticSwitchPin, INPUT_PULLUP);
  pinMode(HeatLedPin, OUTPUT);
  pinMode(ReadyLedPin, OUTPUT);
  pinMode(HeaterPin, OUTPUT);
  pinMode(PumpPin, OUTPUT);
  digitalWrite(HeaterPin, HIGH);
  digitalWrite(PumpPin, HIGH);
  digitalWrite(ReadyLedPin, HIGH);
  waitUntil += Loopdelay;
  waitAutomatic=0;
}


void loop() {
  static int state = S_HEATUP;
  time = millis();
  if ((long)(time - waitUntil) >= 0) {
  if(debug)
  {
    Serial.print("Time:"); Serial.println(time);
  }
  
  uint16_t rtd = max.readRTD();
  float temperature = max.temperature(100, RREF);
  if(temperature<=20.0)
  {
    digitalWrite(ReadyLedPin, HIGH);
    state = S_WAITTILLHOT;
  }
  switch (state)  {
    case S_HEATUP:
      digitalWrite(HeatLedPin, !digitalRead(HeatLedPin));
      state = S_WAITTILLHOT;
      break;
    case S_WAITTILLHOT:
      digitalWrite(HeaterPin, LOW);
      digitalWrite(PumpPin,HIGH);
      if(debug)
      {
        Serial.print("Temperature (WAITTILLHOT) = "); Serial.println(temperature);
      } 
      digitalWrite(HeatLedPin, !digitalRead(HeatLedPin));
      if(temperature>=30.0)
      {
        digitalWrite(HeaterPin, HIGH);
        digitalWrite(HeatLedPin, LOW);
        digitalWrite(ReadyLedPin, LOW);
        state = S_CHECKINPUTS;
      }
      break;
    case S_CHECKINPUTS:
      if(debug)
      {
        Serial.print("Temperature (CHECKINPUTS) = "); Serial.println(temperature);
      }
      if(temperature<=27.0)
      {
        digitalWrite(ReadyLedPin, HIGH);
        state = S_WAITTILLHOT;
      }
      if(digitalRead(AutomaticSwitchPin)==LOW)
      {
        state = S_AUTOMATIC;
        break;
      }
      if (digitalRead(RunSwitchPin)==LOW)
      {
        state = S_RUN;
        break;
      }
      break;
    case S_RUN:
      if(debug)
      {
        Serial.print("Temperature (RUN) = "); Serial.println(temperature);
      }
      if (digitalRead(RunSwitchPin)==LOW)
      {
        digitalWrite(PumpPin,LOW);
        state = S_RUN;
      }
      else
      {
        digitalWrite(PumpPin,HIGH);
        state = S_CHECKINPUTS;
      }     
      break;  
    case S_AUTOMATIC:
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC) = "); Serial.println(temperature);
      }
      FogIntensity = (float) (analogRead(IntensityPin)*(100.0/1023.0));
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC) = "); Serial.println(temperature);
        Serial.print("Intensity (AUTOMATIC) = "); Serial.print(FogIntensity); Serial.print("%");
      }   
      digitalWrite(PumpPin,LOW);
      waitAutomatic=waitUntil+(long)((float)AutomaticTimeMax*FogIntensity);
      state=S_AUTOMATIC_RUN;
      break;   
    case S_AUTOMATIC_RUN:
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC_RUN) = "); Serial.println(temperature);
        Serial.print("Time to Wait (AUTOMATIC_RUN) = "); Serial.print((long)(time-waitAutomatic)); Serial.print("%");
      }   
      if((long)(time-waitAutomatic) >= 0){
        digitalWrite(PumpPin,HIGH);
        state = S_CHECKINPUTS;
      }
      break;
  }
  waitUntil += Loopdelay;  // wait another interval cycle
  }
  //Serial.println();
 // delay(100);
}
