#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>

/* This define statement has to go before the PulseSensor include directive  */
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

#define WINDOW_SIZE 5

void push( int array[], int arr_len, int newest );

/* The digital output pins into which the LCD is connected                   */
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

/* The analog input pin into which the heartbeat sensor is connected         */
const int PULSE_SENSOR_PIN = 0;

/* The time interval over which we want to find the peaks (in milliseconds)  */
const unsigned long interval_millis = 6000;

/* The time since the last update (in milliseconds)                          */
unsigned long previous_update_millis = 0;

/* A length of 240 samples to capture 6s of signal data                      */
const int signal_buffer_length = 240;

/* A buffer array to contain the signal samples over 6s                      */
int signal_buffer[signal_buffer_length] = { 0 };

/* A length of 64 samples to capture the peaks of the signal data            */
const int peak_indices_length = 64;

/* An array to contain the locations of peaks in the signal                  */
int peak_indices[peak_indices_length] = { 0 };

/* A length of 64 samples to capture the peaks of the signal data            */
const int peak_times_length = 64;

/* An array to contain the times of the peaks in the signal                  */
unsigned long peak_times[peak_times_length] = { 0 };

/* The pulse sensor object                                                   */
PulseSensorPlayground pulseSensor;

/* The pulse oximeter object                                                 */
MAX30105 particleSensor;

void setup() {
  Serial.begin( 9600 );

  pulseSensor.analogInput( PULSE_SENSOR_PIN );
  pulseSensor.setThreshold( 1000 );

  if (pulseSensor.begin()) {
    Serial.println( "PulseSensor object created" );
  } /* if */
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
  unsigned long current_update_millis = millis();

  int Signal = analogRead( PULSE_SENSOR_PIN );

  push( signal_buffer, signal_buffer_length, Signal );

  /* If it's been 6 seconds since the last update (ie, a whole new window)  */
  if ( current_update_millis - previous_update_millis > interval_millis ) {
    previous_update_millis = current_update_millis;
    int sum = 0;

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      sum += signal_buffer[i];    
    } /* for */
    
    int baseline = sum / signal_buffer_length;
    int peak_index = 0;
    int peak_value = 0;

    /*                                                                       */
    for ( int i = 0; i < signal_buffer_length; i++ ) {
      if ( signal_buffer[i] > baseline ) {
        if ( peak_value == 0 || signal_buffer[i] > peak_value ) {
          peak_index = i;
          peak_value = signal_buffer[i];
        } /* if */
      } else if ( signal_buffer[i] < baseline && peak_index != 0 ) {
        push( peak_indices, peak_indices_length, peak_index );

        peak_index = 0;
        peak_value = 0;
      } /* if-else */
    } /* for */

    if ( peak_index != 0 ) {
      push( peak_indices, peak_indices_length, peak_index );
    } /* if */
  } /* if */

  delay(25);
}

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arr_len - 1                             *
 * array[] - The array onto which new items will be pushed                   *
 * arr_len - The length of the array                                         *
 * newest - The newest item being pushed onto the array                      */
void push( int array[], int arr_len, int newest ) {
  for ( int i = 0; i < arr_len; i++ ) {
    array[i] = array[i + 1];
  } /* for */
  array[arr_len - 1] = newest;
}
/* EOF */