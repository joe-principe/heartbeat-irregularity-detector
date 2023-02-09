#include "Arduino.h"
#include "helpers.h"

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
  /* If the start and end points are equal, return the value at that point */
  if ( start == end ) { return array[start]; }

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
long mean( const long array[], int start, int end ) {
  /* If the start and end points are equal, return the value at that point */
  if ( start == end ) { return array[start]; }
  
  long sum = 0;
  for ( int i = start; i < end; i++ ) {
    sum += array[i];
  } /* for */
  
  return sum / ( end - start );
} /* End of mean() */

/* EOF */