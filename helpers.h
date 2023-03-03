#ifndef HELPERS_H
#define HELPERS_H

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arr_len - 1                             *
 * array[] - The array onto which new items will be pushed                   *
 * arr_len - The length of the array                                         *
 * newest - The newest item being pushed onto the array                      */
void push( int array[], int arr_len, int newest );

/* Calculates the mean of an array over the interval [start, end)            *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int mean( const int array[], int start, int end );

/* Finds the maximum value of an array over the interval [start, end)        *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
int max( const int array[], int start, int end ); //need to change the name of the function

/* Calculates the mean of an array over the interval [start, end)            *
 * array[] - The array of values                                             *
 * start - The starting point of the interval                                *
 * end - The ending point of the interval                                    */
long mean( const long array[], int start, int end );

#endif /* HELPERS_H */
