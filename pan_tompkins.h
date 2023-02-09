#ifndef PAN_TOMPKINS_H
#define PAN_TOMPKINS_H

/* Header file containing all of the function definitions for the            *
 * Pan-Tompkins algorithm                                                    */

/* Low pass filters the input signal using the equation                      *
 * H(z) = (1 - z^-6)^2 / (1 - z^-1) ^2                                       *
 * signal[] - The mean-shifted signal buffer                                     *
 * signal_length - The length of the signal array                            *
 * lowpass[] - The empty array for the values after filtering                */
void lowPass( const int signal[], const int signal_length, int lowpass[] );

/* High pass filters the low pass filtered signal using the equation         *
 * H(z) = (-1/32 + z^-16 - z^-17 + z^-32 / 32 ) / (1 - z^-1)                 *
 * lowpass[] - The low pass filtered signal                                  *
 * signal_length - The length of the LPF array                               *
 * highpass[] - The empty array for the values after filtering               */
void highPass( const int lowpass[], const int signal_length, int highpass[] );

/* Derivative filters the high pass filtered signal using the equation       *
 * H(z) = 0.1(-z^-2 - 2z^-1 + 2z + z^2)                                      *
 * highpass[] - The high pass filtered signal                                *
 * signal_length - The length of the HPF array                               *
 * derivative[] - The empty array for the values after filtering             */
void derivative( const int highpass[], const int signal_length, int derivative[] );

/* Square step is left in the main function because it is not long enough to *
 * justify having its own function                                           */

/* Filters the squared signal using a moving average window whose length is  *
 * w = 0.15 / T where T is the time between samples in the signal            *
 * square[] - The square of the derivative filtered signal                   *
 * signal_length - The length of the squared array                           *
 * w - The length of the window in samples                                   *
 * moving_average[] - The empty array for the values after filtering         */
void movingAverageFilter( const long square[], const int signal_length, 
                          const int w, long moving_average[] );

#endif /* PAN_TOMPKINS_H */