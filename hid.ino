#include <PulseSensorPlayground.h>
#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>

#define USE_ARDUINO_INTERRUPTS true
#define WINDOW_SIZE 5

void push( int array[], int arr_len, int newest );
int* widenArray( int array[], int arr_len );

/* The digital output pins into which the LCD is connected                   */
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

/* The analog input pin into which the heartbeat sensor is connected         */
const int PULSE_SENSOR_PIN = 0;

/* A length of 240 samples to capture 6s of signal data                      */
const int signal_buffer_length = 240;

/* A buffer array to contain the signal samples over 6s                      */
int signal_buffer[signal_buffer_length] = { 0 };

/* A length of 12 samples to capture the peaks of the signal data            */
int peak_indices_length = 12;

/* An array to contain the locations of peaks in the signal                  */
int peak_indices[peak_indices_length];

/* The value of the current peak                                             */
int peak_value = 0;

PulseSensorPlayground pulseSensor;
MAX30105 particleSensor;

void setup() {
  Serial.begin( 9600 );

  pulseSensor.analogInput( PULSE_SENSOR_PIN );
  pulseSensor.setThreshold( 1000 );

  if (pulseSensor.begin()) {
    Serial.println( "PulseSensor object created" );
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

  int Signal = analogRead( PULSE_SENSOR_PIN );

  push( signal_buffer, signal_buffer_length, Signal );

  int peak_index = 0;  

  for ( int i = 0; i < signal_buffer_length; i++ ) {
    if ( signal_buffer[i] > baseline ) {
      if ( peak_value == 0 || signal_buffer[i] > peak_value ) {
        peak_index = i;
        peak_value = signal_buffer[i];
      }
    } else if ( signal_buffer[i] < baseline && peak_index != 0 ) {
      peak_indices[0] = peak_indices[1];
      peak_indices[1] = peak_index;

      peak_index = 0;
      peak_value = 0;
    }
  }

  if ( peak_index != 0 ) {
    push( peak_indices, peak_indices_length, peak_index );
  }

  delay(25);
}

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arr_len - 1                              *
 * array[] - The array onto which new items will be pushed                   *
 * arr_len - The length of the array                                          *
 * newest - The newest item being pushed onto the array                      */
void push( int array[], int arr_len, int newest ) {
  for ( int i = 0; i < arr_len; i++ ) {
    array[i] = array[i + 1];
  }
  array[arr_len - 1] = newest;
}

/* Doubles an array by returning a new one with twice the length             *
 * array[] - The original array                                              *
 * arr_len - The length of the original array                                 */
int* widenArray( int array[], int arr_len) {
  int new_array[arr_len * 2];

  for ( int i = 0; i < arr_len; i++ ) {
    new_array[i] = array[i];
  }
  
  return new_array;
}
/* EOF */