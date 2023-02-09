#include <heartRate.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Wire.h>

#include "constants.h"
#include "helpers.h"
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
    
    /* The mean-shifted signal */
    int x[signal_buffer_length]; 

    /* The baseline of the input signal */
    int input_baseline = mean( signal_buffer, 0, signal_buffer_length );

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

/* EOF */