#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>


#define USE_ARDUINO_INTERRUPTS true
#define WINDOW_SIZE 5
#include <PulseSensorPlayground.h>
#include <LiquidCrystal.h>
#include <Wire.h>


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int PULSE_SENSOR_PIN = 0;

int Signal;

int INDEX  = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

/* bufLen of 50 to capture >1s of signal data */
int bufLen = 50;
int signalBuffer[50];
/* 2 indices because there will not be more than 2 peaks in 50 values */
int peakIndices[2];
int peakValue = 0;

PulseSensorPlayground pulseSensor;
MAX30105 particleSensor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // lcd.begin(16, 2);

  // lcd.print("BPM: ");

  pulseSensor.analogInput(PULSE_SENSOR_PIN);
  pulseSensor.setThreshold(1000);

  if (pulseSensor.begin()) {
    Serial.println("PulseSensor object created");
  }

  /*if(particleSensor.begin() == false){
    Serial.println("MAX30102 was not found.");
    while(1);
  } */
  // particleSensor.setup();
}
void loop() {

  Signal = analogRead(PULSE_SENSOR_PIN);
  /*VALUE = Signal;
  SUM = SUM - READINGS[INDEX];
  READINGS[INDEX] = VALUE;
  SUM += VALUE;
  INDEX =(INDEX +1) % WINDOW_SIZE;
  AVERAGED = SUM / WINDOW_SIZE;*/

  int myBPM = pulseSensor.getBeatsPerMinute();

  /*lcd.setCursor(5, 0);
  lcd.print("BPM: ");
  lcd.print(myBPM);*/

  // Serial.println(Signal);
  
  //Serial.print(VALUE);
  //Serial.print(",");
  //Serial.println(AVERAGED);

  /*Serial.print(" R[");
	Serial.print(particleSensor.getRed());
	Serial.print("] IR[");
	Serial.print(particleSensor.getIR());
	Serial.println("]");*/

  for ( int i = 0; i < bufLen; i++ ) {
    signalBuffer[i] = signalBuffer[i + 1];
  }
  signalBuffer[49] = Signal;

  int peakIndex = 0;

  int sum = 0;
  for ( int i = 0; i < bufLen; i++ ) {
    sum += signalBuffer[i];
  }
  int baseline = sum / bufLen;

  for ( int i = 0; i < bufLen; i++ ) {
    if ( signalBuffer[i] > baseline ) {
      if ( peakValue == 0 || signalBuffer[i] > peakValue ) {
        peakIndex = i;
        peakValue = signalBuffer[i];
      }
    } else if ( signalBuffer[i] < baseline && peakIndex != 0 ) {
      /* peakIndices.push( peakIndex ); */

      peakIndices[0] = peakIndices[1];
      peakIndices[1] = peakIndex;

      Serial.print("peakIndices[0]: ");
      Serial.print(peakIndices[0]);
      Serial.print(" peakIndices[1]: ");
      Serial.print(peakIndices[1]);
      Serial.println("\n---");
      peakIndex = 0;
      peakValue = 0;
    }
  }

  if ( peakIndex != 0 ) {
    /* peakIndices.push( peakIndex ); */
  }


  delay(25);
  // lcd.clear();
}

