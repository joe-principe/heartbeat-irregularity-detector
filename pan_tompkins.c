#include "Arduino.h"
#include "pan_tompkins.h"

/* C file containing the function definitions for functions in the           *
 * pan_tompkins.h header file                                                */

/* Low pass filters the input signal using the equation                      *
 * H(z) = (1 - z^-6)^2 / (1 - z^-1) ^2                                       *
 * signal[] - The mean-shifted signal buffer                                 *
 * signal_length - The length of the signal array                            *
 * lowpass[] - The empty array for the values after filtering                */
void lowPass( const int signal[], int signal_length, int lowpass[] ) {
  for ( int i = 0; i < signal_length; i++ ) {
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
 * signal_length - The length of the LPF array                               *
 * highpass[] - The empty array for the values after filtering               */
void highPass( const int lowpass[], int signal_length, int highpass[] ) {
  for ( int i = 32; i < signal_length; i++ ) {
    highpass[i] = 32 * lowpass[i - 16] 
                - ( highpass[i - 1] + lowpass[i] - lowpass[i - 32] );
  } /* for */  
} /* End of highPass() */

/* Derivative filters the high pass filtered signal using the equation       *
 * H(z) = 0.1(-z^-2 - 2z^-1 + 2z + z^2)                                      *
 * highpass[] - The high pass filtered signal                                *
 * signal_length - The length of the HPF array                               *
 * derivative[] - The empty array for the values after filtering             */
void derivative( const int highpass[], int signal_length, int derivative[] ) {
    /* signal_length: -2 might need to be -3 instead */
    for ( int i = 2; i < signal_length - 2; i++ ) {
      double temp = ( 1 / ( 8 * Ts_millis ) ) 
                  * ( -highpass[i - 2] - 2 * highpass[i - 1] 
                  + 2 * highpass[i + 1] + highpass[i + 2] );
      temp = round( temp );
      derivative[i] = ( int )temp;
    } /* for */
} /* End of derivative() */

/* Square step is left in the main function because it is not long enough to *
 * justify having its own function                                           */

/* Filters the squared signal using a moving average window whose length is  *
 * w = 0.15 / T where T is the time between samples in the signal            *
 * square[] - The square of the derivative filtered signal                   *
 * signal_length - The length of the squared array                           *
 * w - The length of the window in samples                                   *
 * moving_average[] - The empty array for the values after filtering         */
void movingAverageFilter( const long square[], int signal_length, int w,
                          long moving_average[] ) {        
  for ( int i = w; i < signal_length; i++ ) {
    moving_average[i] = mean( square, i, i + w - 1 );
    if ( i < 2 * w ) { moving_average[i - w] = moving_average[w]; }
  } /* for */
} /* End of movingAverageFilter() */