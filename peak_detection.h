#ifndef PEAK_DETECTION_H
#define PEAK_DETECTION_H

#include "constants.h"

/* Finds the indices of the peaks of the signal                              *
 * Returns the number of peaks within the signal                             *
 * signal[] - The array of the moving average filtered signal                *
 * peak_indices - The empty array of indices of the peaks                    */
int findPeaks( const long signal[], int peak_indices[] );

/* Calculates the heart rate variability (HRV) using the root mean square of *
 * successive differences.                                                   *
 * peak_indices[] - The array of the indices of the peaks of the signal      *
 * num_peaks - The number of peaks in the signal                             */
float RMSSD( const int peak_indices[], int num_peaks );

#endif /* PEAK_DETECTION_H */