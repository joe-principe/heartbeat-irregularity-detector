#ifndef CONSTANTS_H
#define CONSTANTS_H

/* The register select line for the LCD */
const int rs = 12;

/* The enable line for the LCD */
const int en = 11;

/* The D4 bit (bit 5) for the LCD */
const int d4 = 5;

/* The D5 bit (bit 6) for the LCD */
const int d5 = 4;

/* The D6 bit (bit 7) for the LCD */
const int d6 = 3;

/* The D7 bit (bit 8) for the LCD 8 */
const int d7 = 2;

/* The analog input pin into which the heartbeat sensor is connected */
const int PULSE_SENSOR_PIN = 0;

/* The time interval over which we want to find the peaks (in milliseconds) */
const unsigned long interval_millis = 6000;

/* A length of 240 samples to capture 6 seconds of signal data */
const int signal_buffer_length = 240;

/* A length of 64 samples to capture the peaks of the signal data */
const int peak_indices_length = 64;

/* A length of 64 samples for the successive differences of the peaks */
const int sd_length = 64;

/* The time between samples in milliseconds */
const int Ts_millis = 25;

#endif /* CONSTANTS_H */