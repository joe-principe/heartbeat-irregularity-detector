/* Style guide
 * Pre-processor definitions and macros use MACRO_CASE                       *
 * Custom types use PascalCase                                               *
 * Functions use camelCase                                                   *
 * Variables use snake_case                                                  *
 * No space between function name and open parenthesis, func() not func ()   *
 * Space after every open parenthesis and before every close parenthesis     *
 * Space after every open bracket and before every close bracket             *
 * Space between control flow statement and open parenthesis, eg for ()      *
 * Comment after control flow statements to indicate end of statement        *
 * K&R style brackets, ie open bracket is on same line as control statement  *
 * Define function and its arguments in comment above the function           */

#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>
#include <math.h>

/* This define statement has to go before the PulseSensor include directive  */
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

#define WINDOW_SIZE 5

void push( int array[], int arr_len, int newest );
int mean( int array[], int start, int end );

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
  Serial.begin ( 9600 );

  pulseSensor.analogInput( PULSE_SENSOR_PIN );
  pulseSensor.setThreshold( 1000 );

  if ( pulseSensor.begin() ) {
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

  int signal = analogRead( PULSE_SENSOR_PIN );

  push( signal_buffer, signal_buffer_length, signal );

  /* If it's been 6 seconds since the last update (ie, a whole new window) */
  if ( current_update_millis - previous_update_millis > interval_millis ) {
    /* Stores the sampling frequency of the signal (in hertz) */
    int fs = 240;
    
    /* Stores the period of the signal (in seconds) */
    float T = 1 / ( float )fs; 
    
    int x[signal_buffer_length]; /* Stores the mean-shifted signal */

    int baseline = mean( signal_buffer, 0, signal_buffer_length );

    /* Mean shift the signal */
    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x = x[i] - baseline;
    } /* for */

    /* Low Pass Filter */
    int x_LP[signal_buffer_length];

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      if ( i < 12 ) {
        x_LP[i] = x[i];
      } else {
        x_LP[i] 2 * x_LP[i - 1] - x_LP[i - 2] + x[i] - 2 * x[i - 6] + x[i - 12];
      } /* if-else */
    } /* for */

    /* High Pass Filter */
    int x_HP[signal_buffer_length];

    for ( int i = 32; i < signal_buffer_length; i++ ) {
      x_HP[i] = 32 * x_LP[i - 16] - ( x_HP[i - 1] + x_LP[i] - x_LP[i - 32] );
    } /* for */

    /* Derivative */
    int x_D[signal_buffer_length];

    for ( int i = 2; i < signal_buffer_length; i++ ) {
      double temp = ( 1 / ( 8 * T ) ) * ( -x_HP[i - 2] - 2 * x_HP[i - 1] + 2 * x_HP[i + 1] + x_HP[i + 2] );
      temp = round( temp );
      int x_D = ( int )temp;
    } /* for */

    /* Square */    
    long x_S[signal_buffer_length];

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x_S[i] = x_D[i] * x_D[i];
    } /* for */

    /* Moving Average Filter */
    int w = 0.150 / T;
    int moving_average_length = signal_buffer_length - w + 1;
    long x_MAF[moving_average_length + w];

    for ( int i = w; i < moving_average_length + w; i++ ) {
      x_MAF[i] = mean( x_S, i, i + w - 1 );
      if ( i < 2 * w ) {
        x_MAF[i - w] = x_MAF[w];
      } /* if */
    } /* for */

    /* Peak Detection */
  } /* if */

  delay(25);
} /* End of main() */

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arr_len - 1                             *
 * array[] - The array onto which new items will be pushed                   *
 * arr_len - The length of the array                                         *
 * newest - The newest item being pushed onto the array                      */
void push(int array[], int arr_len, int newest) {
  for ( int i = 0; i < arr_len; i++ ) {
    array[i] = array[i + 1];
  } /* for */
  array[arr_len - 1] = newest;
} /* End of push() */

/* Calculates the mean of an array over the interval [start, end)            *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int mean( const int array[], int start, int end ) {
  int sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */
  return sum / ( end - start );
} /* End of mean() */
/* EOF */