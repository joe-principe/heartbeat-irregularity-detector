#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>
#include <math.h>

/* This define statement has to go before the PulseSensor include directive  */
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

void push( int array[], int arr_len, int newest );
int mean( int array[], int start, int end );
int max( int array[], int start, int end );
long mean( long array[], int start, int end );

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
int signal_buffer[signal_buffer_length];

/* A length of 64 samples to capture the peaks of the signal data            */
const int peak_indices_length = 64;

/* An array to contain the locations of peaks in the signal                  */
int peak_indices[peak_indices_length];

/* The number of peaks within the window                                     */
int num_peaks = 0;

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

void loop() {
  unsigned long current_update_millis = millis();

  int signal = analogRead( PULSE_SENSOR_PIN );

  push( signal_buffer, signal_buffer_length, signal );

  /* If it's been 6 seconds since the last update (ie, a whole new window) */
  if ( current_update_millis - previous_update_millis > interval_millis ) {
    previous_update_millis = current_update_millis;

    /* Stores the number of samples in the time interval */
    int fs = 240;
    
    /* Stores the period of the signal (in seconds) */
    float T = 1 / ( float )fs; 
    
    /* Stores the mean-shifted signal */
    int x[signal_buffer_length]; 

    /* Stores the baseline of the input signal */
    int input_baseline = mean( signal_buffer, 0, signal_buffer_length );

    /***                      Mean shift the signal                        ***/
    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x = x[i] - input_baseline;
    } /* for */

    /***                         Low Pass Filter                           ***/
    int x_LP[signal_buffer_length];

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      if ( i < 12 ) {
        x_LP[i] = x[i];
      } else {
        x_LP[i] = 2 * x_LP[i - 1] - x_LP[i - 2] 
                + x[i] - 2 * x[i - 6] + x[i - 12];
      } /* if-else */
    } /* for */

    /***                        High Pass Filter                           ***/
    int x_HP[signal_buffer_length];

    for ( int i = 32; i < signal_buffer_length; i++ ) {
      x_HP[i] = 32 * x_LP[i - 16] - ( x_HP[i - 1] + x_LP[i] - x_LP[i - 32] );
    } /* for */

    /***                           Derivative                              ***/
    int x_D[signal_buffer_length];

    /* signal_buffer_length - 2 might need to be - 3 instead */
    for ( int i = 2; i < signal_buffer_length - 2; i++ ) {
      double temp = ( 1 / ( 8 * T ) ) 
                  * ( -x_HP[i - 2] - 2 * x_HP[i - 1] 
                  + 2 * x_HP[i + 1] + x_HP[i + 2] );
      temp = round( temp );
      int x_D = ( int )temp;
    } /* for */

    /***                              Square                               ***/    
    long x_S[signal_buffer_length];

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x_S[i] = x_D[i] * x_D[i];
    } /* for */

    /***                       Moving Average Filter                       ***/
    int w = 0.150 / T;
    long x_MAF[signal_buffer_length];

    for ( int i = w; i < signal_buffer_length; i++ ) {
      x_MAF[i] = mean( x_S, i, i + w - 1 );
      if ( i < 2 * w ) {
        x_MAF[i - w] = x_MAF[w];
      } /* if */
    } /* for */

    /***                          Peak Detection                           ***/
    int peak_index = 0;
    int peak_value = 0;

    /* Stores the baseline of the mean-average filtered signal */
    long maf_baseline = mean( x_MAF, 0, signal_buffer_length );

    /* Finds the indices of the peaks of the signal */
    /* After a peak has been found, if the current value dips below the      *
     * baseline, then peak_index gets pushed onto the array and we start     *
     * looking for a new peak                                                */
    for ( int i = 0; i < signal_buffer_length; i++ ) {
      if ( x_MAF[i] > maf_baseline ) {
        if ( peak_value == 0 || x_MAF[i] > peak_value ) {
          peak_index = i;
          peak_value = x_MAF[i];
        } /* if */
      } else if ( x_MAF[i] <= maf_baseline && peak_index != 0 ) {
        push( peak_indices, peak_indices_length, peak_index );

        peak_index = 0;
        peak_value = 0;
        num_peaks++;
      } /* if-else */
    } /* for */

    /* In case we didn't dip below baseline after finding a peak at the end  */
    if ( peak_index != 0 ) {
      push( peak_indices, peak_indices_length, peak_index );
    } /* if */

    /***                        Find Peak Intervals                        ***/
    for ( int i = 0; i < peak_indices_length - 1; i++ ) {
      int interbeat_interval = 25 * peak_indices[i + 1] - peak_indices[i];
      push( sd, sd_length, interbeat_interval );        
    } /* for */    

    /***                       Calculate Variability                       ***/
    int sd_sum = 0;    
    for ( int i = 0; i < sd_length; i++ ) {
      sd_square[i] = sd[i] * sd[i];
      sd_sum += sd_square[i] / num_peaks;
    } /* for */
    float sd_rmssd = sqrt( sd_sum );
  } /* if */

  delay( 25 );
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
int mean( const int array[], int start, int end ) {
  int sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */
  return sum / ( end - start );
} /* End of mean() */

/* Finds the maximum value of an array over the interval [start, end)        *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int max( const int array[], int start, int end ) {
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
long mean( const long array[], int start, int end ) {
  long sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */
  return sum / ( end - start );
} /* End of mean() */
/* EOF */