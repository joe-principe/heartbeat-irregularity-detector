#include <heartRate.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Wire.h>

#include "constants.h"
#include "helpers.h"
#include "pan_tompkins.h"

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

/* The LCD object */
LiquidCrystal lcd( rs, en, d4, d5, d6, d7 );

/* The pulse sensor object */
PulseSensorPlayground pulseSensor;

/* The pulse oximeter object */
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
    lowPass( x, signal_buffer_length, x_LP );

    /***                        High Pass Filter                           ***/
    int x_HP[signal_buffer_length];
    highPass( x_LP, signal_buffer_length, x_HP );

    /***                           Derivative                              ***/
    int x_D[signal_buffer_length];
    derivative( x_HP, signal_buffer_length, x_D );

    /***                              Square                               ***/    
    long x_S[signal_buffer_length];

    for ( int i = 0; i < signal_buffer_length; i++ ) {
      x_S[i] = x_D[i] * x_D[i];
    } /* for */

    /***                       Moving Average Filter                       ***/
    int w = 0.150 / Ts_millis;
    long x_MAF[signal_buffer_length];
    movingAverageFilter( x_S, signal_buffer_length, w, x_MAF );

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
      num_peaks++;
    } /* if */

    /***            Find Peak Intervals (Successive Differences)           ***/
    for ( int i = 0; i < peak_indices_length - 1 && peak_indices[i] != 0; i++ ) {
      int interbeat_interval = 25 * peak_indices[i + 1] - peak_indices[i];
      push( sd, sd_length, interbeat_interval );        
    } /* for */    

    /***                   Calculate Variability (RMSSD)                   ***/
    int sd_sum = 0;
    
    /* Finds the square of the successive differences */
    for ( int i = 0; i < sd_length; i++ ) {
      sd_square += sd[i] * sd[i];
    } /* for */

    /* Finds the mean of the square of the successive differences */
    int sd_avg = sd_square / num_peaks;    

    /* Finds the Root of the MSSD */
    float sd_rmssd = sqrt( sd_avg );
  } /* if */

  delay( 25 );
} /* End of main() */

/* EOF */