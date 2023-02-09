#include "Arduino.h"
#include "peak_detection.h"

/* Finds the indices of the peaks of the signal                              *
 * Returns the number of peaks within the signal                             *
 * signal[] - The array of the moving average filtered signal                *
 * peak_indices - The empty array of the indices of the peaks                */
int findPeaks( const long signal[], int peak_indices[] ) {
  /* After a peak has been found, if the current value dips below the        *
   * baseline, then peak_index gets pushed onto the array and we start       *
   * looking for a new peak                                                  */
  
  int peak_index = 0, peak_value = 0, num_peaks = 0;
  
  long baseline = mean( signal, 0, signal_buffer_length );
  
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