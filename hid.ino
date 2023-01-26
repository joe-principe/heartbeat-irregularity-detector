#include <PulseSensorPlayground.h>
#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>

#define USE_ARDUINO_INTERRUPTS true
#define WINDOW_SIZE 5

void Push( int array[], int arrLen, int newest );

/* The digital output pins into which the LCD is connected                   */
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

/* The analog input pin into which the heartbeat sensor is connected         */
const int PULSE_SENSOR_PIN = 0;

/* A length of 240 samples to capture 6s of signal data                      */
int signalBufferLength = 240;

/* A buffer array to contain the signal samples over 6s                      */
int signalBuffer[signalBufferLength] = { 0 };

/* A length of 12 samples to capture the peaks of the signal data            */
int peakIndicesLength = 12;

/* An array to contain the locations of peaks in the signal                  */
int peakIndices[peakIndicesLength];

/* The value of the current peak                                             */
int peakValue = 0;

PulseSensorPlayground pulseSensor;
MAX30105 particleSensor;

void setup() {
  Serial.begin(9600);

  pulseSensor.analogInput(PULSE_SENSOR_PIN);
  pulseSensor.setThreshold(1000);

  if (pulseSensor.begin()) {
    Serial.println("PulseSensor object created");
  }
}

/* 1: Find the peaks of the signal over a time window                        *
 * The longer the buffer length, the longer the time window. Longer time     *
 * windows will give more peaks, which could help for better irregularity    *
 * detection                                                                 *
 *                                                                           *
 * 2: Calculate the time between the peaks of the signal. This will be the   *
 * key to finding tachycardia and bradycardia in the signal                  *
 *                                                                           *
 * 3: Figure out how to detect irregularities with the SpO2 values from the  *
 * oximeter                                                                  *
 *                                                                           *
 * 4: Profit B)                                                              */
void loop() {

  int Signal = analogRead(PULSE_SENSOR_PIN);

  Push( signalBuffer, signalBufferLength, Signal );

  int peakIndex = 0;

  int sum = 0;
  for ( int i = 0; i < signalBufferLength; i++ ) {
    sum += signalBuffer[i];
  }
  int baseline = sum / signalBufferLength;

  for ( int i = 0; i < signalBufferLength; i++ ) {
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
    Push( peakIndices, peakIndicesLength, peakIndex );
  }

  delay(25);
}

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arrLen - 1                              */
void Push( int array[], int arrLen, int newest ) {
  for ( int i = 0; i < arrLen; i++ ) {
    array[i] = array[i + 1];
  }
  array[arrLen - 1] = newest;
}
/* EOF */