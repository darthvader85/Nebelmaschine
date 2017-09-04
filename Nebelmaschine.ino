/***************************************************
  This is a Script to Control a Fog machine with a
  heater, a pump and a Type K Thermocouple as Temperaturesensor.

  The control Panel has to Switches as Input (Run, Automatic)
  and a Variable Resistor to controll intensity.

  There are two LED´s to inditcate Status. Heat and ready.
 ****************************************************/

#include <Adafruit_MAX31855.h>
#define Loopdelay 100
#define Blinkdelay 1000
#define AutomaticTimeMax 60000UL

#define MAXDO   2
#define MAXCS   3
#define MAXCLK  4

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
const float Temp_min=180;
const float Temp_max=300;
int blink_Counter=0;

unsigned long waitUntil=0;
unsigned long waitAutomatic=0;
unsigned long waitBlink=0;

const int debug=1;
long FogIntensity ;
unsigned long time;

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

void setup() {
  Serial.begin(115200);
  Serial.println("Controller for SLE Cyberfog Fog Machine");
  Serial.print("Internal Temp = ");
  Serial.println(thermocouple.readInternal());

  double c = thermocouple.readCelsius();
  if (isnan(c)) {
    Serial.println("Something wrong with thermocouple!");
  } else {
    Serial.print("C = "); 
    Serial.println(c);
  }

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
  waitBlink += Blinkdelay;
}


void loop() {
  static int state = S_WAITTILLHOT;
  time = millis();
  if ((long)(time - waitUntil) >= 0) {
  if(debug)
  {
    Serial.print("Time:"); Serial.println(time);
  }
  
  double temperature = thermocouple.readCelsius();
  if(temperature<=Temp_min)
  {
    digitalWrite(ReadyLedPin, HIGH);
    state = S_WAITTILLHOT;
  }
  switch (state)  {
    case S_WAITTILLHOT:
      digitalWrite(HeaterPin,LOW);
      digitalWrite(PumpPin,HIGH);
      if(debug)
      {
        Serial.print("Temperature (WAITTILLHOT) = "); Serial.println(temperature);
      } 
      if((long)(time - waitBlink) >= 0)
      {
        waitBlink += Blinkdelay;
        digitalWrite(HeatLedPin, !digitalRead(HeatLedPin));
      }
      if(temperature>=Temp_max)
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
      if(temperature<=Temp_min)
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
        digitalWrite(HeaterPin,LOW);
        digitalWrite(PumpPin,LOW);
        state = S_RUN;
      }
      else
      {
        digitalWrite(HeaterPin,HIGH);
        digitalWrite(PumpPin,HIGH);
        state = S_CHECKINPUTS;
      }     
      break;  
    case S_AUTOMATIC:
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC) = "); Serial.println(temperature);
      }
      FogIntensity = (long)analogRead(IntensityPin)*10;
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC) = "); Serial.println(temperature);
        Serial.print("Intensity (AUTOMATIC) = "); Serial.print(FogIntensity); Serial.print("%");
      }   
      digitalWrite(PumpPin,LOW);
      digitalWrite(HeaterPin,LOW);
      waitAutomatic=waitUntil+FogIntensity;
      state=S_AUTOMATIC_RUN;
      break;   
    case S_AUTOMATIC_RUN:
      if(debug)
      {
        Serial.print("Temperature (AUTOMATIC_RUN) = "); Serial.println(temperature);
        Serial.print("Time to Wait (AUTOMATIC_RUN) = "); Serial.print((long)(time-waitAutomatic)); Serial.print("%");
      }   
      if(digitalRead(AutomaticSwitchPin)==LOW)
      {
        if((long)(time-waitAutomatic) >= 0){
          digitalWrite(PumpPin,HIGH);
          state = S_CHECKINPUTS;
        }
      }
      else
      {
          digitalWrite(HeaterPin,HIGH);
          digitalWrite(PumpPin,HIGH);
          state = S_CHECKINPUTS;
      }
      break;
  }
  waitUntil += Loopdelay;  // wait another interval cycle
  }
}
