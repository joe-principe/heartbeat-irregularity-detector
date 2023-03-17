#include <heartRate.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Wire.h>

#include "pan_tompkins.h"
#include "peak_detection.h"

/* This define statement has to go before the PulseSensor include directive  */
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

/* A buffer to contain the signal samples over 6 seconds                     */
int signal_buffer[signal_buffer_length];

/* An array to contain the locations of peaks in the signal                  */
int peak_indices[peak_indices_length];

/* The number of peaks within the window                                     */
int num_peaks = 0;

/* An array to contain the interbeat intervals of the signal                 */
int sd[sd_length];

/* The time since the last update (in milliseconds)                          */
unsigned long previous_update_millis = 0;

/* The register select line for the LCD */
const int rs = 12;

/* The enable line for the LCD */
const int en = 11;

/* The D4 bit (bit 5) for the LCD */
const int d4 = 5;

/* The D5 bit (bit 6) for the LCD */
const int d5 = 4;

/* The D6 bit (bit 7) for the LCD */
const int d6 = 3;

/* The D7 bit (bit 8) for the LCD 8 */
const int d7 = 2;

/* The analog input pin into which the heartbeat sensor is connected */
const int PULSE_SENSOR_PIN = 0;

/* The time interval over which we want to find the peaks (in milliseconds) */
const unsigned long interval_millis = 6000;

/* A length of 240 samples to capture 6 seconds of signal data */
const int signal_buffer_length = 240;

/* A length of 64 samples to capture the peaks of the signal data */
const int peak_indices_length = 64;

/* A length of 64 samples for the successive differences of the peaks */
const int sd_length = 64;

/* The time between samples in milliseconds */
const int Ts_millis = 25;

/* The LCD object */
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

/* The pulse sensor object */
PulseSensorPlayground pulseSensor;

/* The pulse oximeter object */
MAX30105 particleSensor;

void push( int array[], int arr_len, int newest );

int meani( const int array[], int start, int end );

int maxi( const int array[], int start, int end );

long meanl( const long array[], int start, int end );

void lowPass( const int signal[], int lowpass[] );

void highPass( const int lowpass[], int highpass[] );

void derivative( const int highpass[], int derivative[] );

void squareSignal( const int derivative[], long square[] );

void movingAverageFilter( const long square[], int w, long moving_average[] );

int findPeaks( const long signal[], int peak_indices[] );

float RMSSD( const int peak_indices[], int num_peaks );

void setup() {
  Serial.begin ( 9600 );

  pulseSensor.analogInput( PULSE_SENSOR_PIN );
  pulseSensor.setThreshold( 1000 );

  if ( pulseSensor.begin() ) {
    Serial.println( "PulseSensor object created" );
  } /* if */
}

void loop() {
  unsigned long current_update_millis = millis();

  int signal = analogRead( PULSE_SENSOR_PIN );

  push( signal_buffer, signal_buffer_length, signal );

  /* If it's been 6 seconds since the last update (ie, a whole new window) */
  if ( current_update_millis - previous_update_millis > interval_millis ) {
    previous_update_millis = current_update_millis;
    
    /* The mean-shifted signal */
    int x[signal_buffer_length]; 

    /* The baseline of the input signal */
    int input_baseline = meani( signal_buffer, 0, signal_buffer_length );

    /*** Mean shift the signal ***/
    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x[i] = x[i] - input_baseline;
    } /* for */

    /*** Low Pass Filter ***/
    int x_LP[signal_buffer_length];
    lowPass( x, x_LP );

    /*** High Pass Filter ***/
    int x_HP[signal_buffer_length];
    highPass( x_LP, x_HP );

    /*** Derivative ***/
    int x_D[signal_buffer_length];
    derivative( x_HP, x_D );

    /*** Square ***/    
    long x_S[signal_buffer_length];
    squareSignal( x_D, x_S );

    /*** Moving Average Filter ***/
    int w = 0.150 / Ts_millis;
    long x_MAF[signal_buffer_length];
    movingAverageFilter( x_S, w, x_MAF );

    /*** Peak Detection ***/
    num_peaks = findPeaks( x_MAF, peak_indices );
    
    /*** HRV ***/
    float rmssd = RMSSD( peak_indices[], num_peaks );
  } /* if */

  delay( Ts_millis );
} /* End of main() */

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
} /* End of push() */

/* Calculates the mean of an array over the interval [start, end)            *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int meani( const int array[], int start, int end ) {
  /* If the start and end points are equal, return the value at that point */
  if ( start == end ) { return array[start]; }

  int sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */

  return sum / ( end - start );
} /* End of meani() */

/* Finds the maximum value of an array over the interval [start, end)        *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int maxi( const int array[], int start, int end ) {
  /* If the start and end points are equal, return the value at that point */
  if ( start == end ) { return array[start]; }

  int max = 0;
  for ( int i = start; i < end; i++ ) {
    if ( array[i] > max ) { max = array[i]; }
  } /* for */

  return max;
} /* End of max() */

/* Calculates the mean of an array over the interval [start, end)            *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
long meanl( const long array[], int start, int end ) {
  /* If the start and end points are equal, return the value at that point */
  if ( start == end ) { return array[start]; }
  
  long sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */
  
  return sum / ( end - start );
} /* End of meanl() */

/* Low pass filters the input signal using the equation                      *
 * H(z) = (1 - z^-6)^2 / (1 - z^-1) ^2                                       *
 * signal[] - The mean-shifted signal buffer                                 *
 * signal_buffer_length - The length of the signal array                            *
 * lowpass[] - The empty array for the values after filtering                */
void lowPass( const int signal[], int signal_buffer_length, int lowpass[] ) {
  for ( int i = 0; i < signal_buffer_length; i++ ) {
    if ( i < 12 ) {
      lowpass[i] = signal[i];
    } else {
      lowpass[i] = 2 * lowpass[i - 1] - lowpass[i - 2] 
              + signal[i] - 2 * signal[i - 6] + signal[i - 12];
    } /* if-else */
  } /* for */
} /* End of lowPass() */

/* High pass filters the low pass filtered signal using the equation         *
 * H(z) = (-1/32 + z^-16 - z^-17 + z^-32 / 32 ) / (1 - z^-1)                 *
 * lowpass[] - The low pass filtered signal                                  *
 * highpass[] - The empty array for the values after filtering               */
void highPass( const int lowpass[], int highpass[] ) {
  for ( int i = 32; i < signal_buffer_length; i++ ) {
    highpass[i] = 32 * lowpass[i - 16] 
                - ( highpass[i - 1] + lowpass[i] - lowpass[i - 32] );
  } /* for */  
} /* End of highPass() */

/* Derivative filters the high pass filtered signal using the equation       *
 * H(z) = 0.1(-z^-2 - 2z^-1 + 2z + z^2)                                      *
 * highpass[] - The high pass filtered signal                                *
 * derivative[] - The empty array for the values after filtering             */
void derivative( const int highpass[], int derivative[] ) {
    /* signal_buffer_length: -2 might need to be -3 instead */
    for ( int i = 2; i < signal_buffer_length - 2; i++ ) {
      double temp = ( 1 / ( 8 * Ts_millis ) ) 
                  * ( -highpass[i - 2] - 2 * highpass[i - 1] 
                  + 2 * highpass[i + 1] + highpass[i + 2] );
      temp = round( temp );
      derivative[i] = ( int )temp;
    } /* for */
} /* End of derivative() */

/* Square step is left in the main function because it is not long enough to *
 * justify having its own function                                           */
void squareSignal( const int derivative[], long square[] ) {
  for ( int i = 0; i < signal_buffer_length; i++ ) {
    square[i] = derivative[i] * derivative[i];
  } /* for */
} /* End of squareSignal() */

/* Filters the squared signal using a moving average window whose length is  *
 * w = 0.15 / T where T is the time between samples in the signal            *
 * square[] - The square of the derivative filtered signal                   *
 * w - The length of the window in samples                                   *
 * moving_average[] - The empty array for the values after filtering         */
void movingAverageFilter( const long square[], int w, long moving_average[] ) {        
  for ( int i = w; i < signal_buffer_length; i++ ) {
    moving_average[i] = meanl( square, i, i + w - 1 );
    if ( i < 2 * w ) { moving_average[i - w] = moving_average[w]; }
  } /* for */
} /* End of movingAverageFilter() */

/* Finds the indices of the peaks of the signal                              *
 * Returns the number of peaks within the signal                             *
 * signal[] - The array of the moving average filtered signal                *
 * peak_indices - The empty array of the indices of the peaks                */
int findPeaks( const long signal[], int peak_indices[] ) {
  /* After a peak has been found, if the current value dips below the        *
   * baseline, then peak_index gets pushed onto the array and we start       *
   * looking for a new peak                                                  */
  
  int peak_index = 0, peak_value = 0, num_peaks = 0;
  
  long baseline = meanl( signal, 0, signal_buffer_length );
  
  for ( int i = 0; i < signal_buffer_length; i++ ) {
    if ( signal[i] > baseline ) {
      if ( peak_value == 0 || signal[i] > peak_value ) {
        peak_index = i;
        peak_value = signal[i];
      } /* if */
    } else if ( signal[i] <= baseline && peak_index != 0 ) {
      push( peak_indices, peak_indices_length, peak_index );
      peak_index = 0;
      peak_value = 0;
      num_peaks++;
    } /* if-else */
  } /* for */

  /* In case we didn't dip below baseline after finding a peak at the end  */
  if ( peak_index != 0 ) {
    push( peak_indices, peak_indices_length, peak_index );
    num_peaks++;
  } /* if */

  return num_peaks;
} /* End of findPeaks() */

/* Calculates the heart rate variability (HRV) using the root mean square of *
 * successive differences.                                                   *
 * peak_indices[] - The array of the indices of the peaks of the signal      *
 * num_peaks - The number of peaks in the signal                             */
float RMSSD( const int peak_indices[], int num_peaks ) {
  long sd_square = 0;
  
  /* Finds the square of the successive differences */
  for ( int i = 0; i < num_peaks - 1; i++ ) {
    int ibi = Ts_millis * ( peak_indices[i + 1] - peak_indices[i] );
    sd_square += ibi * ibi;
  } /* for */

  /* Finds the mean of the square of the successive differences */
  double sd_avg = sd_square / num_peaks;    

  return ( float )sqrt( sd_avg );
} /* End of RMSSD() */

/* EOF */