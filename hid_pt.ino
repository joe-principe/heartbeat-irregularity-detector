#include <spo2_algorithm.h>
#include <LiquidCrystal.h>
#include <heartRate.h>
#include <MAX30105.h>
#include <Wire.h>
#include <math.h>

/* This define statement has to go before the PulseSensor include directive  */
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

#define WINDOW_SIZE 5

void push(int array[], int arr_len, int newest);

/* The digital output pins into which the LCD is connected                   */
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* The analog input pin into which the heartbeat sensor is connected         */
const int PULSE_SENSOR_PIN = 0;

/* The time interval over which we want to find the peaks (in milliseconds)  */
const unsigned long interval_millis = 6000;

/* The time since the last update (in milliseconds)                          */
unsigned long previous_update_millis = 0;

/* A length of 240 samples to capture 6s of signal data                      */
const int signal_buffer_length = 240;

/* A buffer array to contain the signal samples over 6s                      */
int signal_buffer[signal_buffer_length] = { 0 };

/* A length of 64 samples to capture the peaks of the signal data            */
const int peak_indices_length = 64;

/* An array to contain the locations of peaks in the signal                  */
int peak_indices[peak_indices_length] = { 0 };

/* A length of 64 samples to capture the peaks of the signal data            */
const int peak_times_length = 64;

/* An array to contain the times of the peaks in the signal                  */
unsigned long peak_times[peak_times_length] = { 0 };

/* The pulse sensor object                                                   */
PulseSensorPlayground pulseSensor;

/* The pulse oximeter object                                                 */
MAX30105 particleSensor;

void setup() {
  Serial.begin(9600);

  pulseSensor.analogInput(PULSE_SENSOR_PIN);
  pulseSensor.setThreshold(1000);

  if (pulseSensor.begin()) {
    Serial.println("PulseSensor object created");
  } /* if */
}

/* 1: Find the peaks of the signal over a time window                        *
 * The longer the buffer length, the longer the time window. Longer time     *
 * windows will give more peaks, which could help for better irregularity    *
 * detection                                                                 *
 *                                                                           *
 * 2: Calculate the time between the peaks of the signal. This will be the   *
 * key to finding tachycardia and bradycardia in the signal                  *
 *                                                                           *
 * 3: Figure out how to detect irregularities with the SpO2 values from the  *
 * oximeter                                                                  *
 *                                                                           *
 * 4: Profit B)                                                              */
void loop() {
  unsigned long current_update_millis = millis();

  int Signal = analogRead(PULSE_SENSOR_PIN);

  push(signal_buffer, signal_buffer_length, Signal);

  /* If it's been 6 seconds since the last update (ie, a whole new window)  */
  if (current_update_millis - previous_update_millis > interval_millis) {
    int fs = 240;
    float T = 1 / ( float )fs;

    int x[signal_buffer_length];
    int sum = 0;
    for (int i = 0; i < signal_buffer_length; i++) {
      sum += signal_buffer[i];
      x[i] = signal_buffer[i];
    }
    int baseline = sum / signal_buffer_length;

    /* Mean shift the signal */
    for (int i = 0; i < signal_buffer_length; i++) {
      x = x[i] - baseline;
    }

    /* Low Pass Filter */
    int x_LP[signal_buffer_length];
    for (int i = 0; i < signal_buffer_length; i++) {
      if (i < 12) {
        x_LP[i] = x[i];
      } else {
        x_LP[i] 2 * x_LP[i - 1] - x_LP[i - 2] + x[i] - 2 * x[i - 6] + x[i - 12];
      }
    }

    /* High Pass Filter */
    int x_HP[signal_buffer_length];
    for (int i = 32; i < signal_buffer_length; i++) {
      x_HP[i] = 32 * x_LP[i - 16] - (x_HP[i - 1] + x_LP[i] - x_LP[i - 32]);
    }

    /* Derivative */
    int x_D[signal_buffer_length];
    for (int i = 2; i < signal_buffer_length; i++) {
      double temp = (1 / (8 * T)) * (-x_HP[i - 2] - 2 * x_HP[i - 1] + 2 * x_HP[i + 1] + x_HP[i + 2]);
      temp = round(temp);
      int x_D = (int)temp;
    }

    long x_S[signal_buffer_length];
    for (int i = 0; i < signal_buffer_length; i++) {
      x_S[i] = x_D[i] * x_D[i];
    }

    int w = 0.150 / T;
    int moving_average_length = signal_buffer_length - w + 1;
    int x_MAF[moving_average_length];
    for ( int i = 0; i < moving_average_length; i++ ) {
      for ()
    }
  } /* if */

  delay(25);
}

/* Pushes a new item onto the array while removing the oldest item           *
 * Oldest @ index 0, newest at index arr_len - 1                             *
 * array[] - The array onto which new items will be pushed                   *
 * arr_len - The length of the array                                         *
 * newest - The newest item being pushed onto the array                      */
void push(int array[], int arr_len, int newest) {
  for (int i = 0; i < arr_len; i++) {
    array[i] = array[i + 1];
  } /* for */
  array[arr_len - 1] = newest;
}
/* EOF */