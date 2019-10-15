#ifndef BINCLOCK_H
#define BINCLOCK_H

//Some reading (if you want)
//https://stackoverflow.com/questions/1674032/static-const-vs-define-vs-enum

// Function definitions

int hexCompensation(int units);
int decCompensation(int units);
void initGPIO(void);
void cleanUp(void);
void int_to_bin_digit(unsigned int in, int count, int* out);

int hFormat(int hours);
void lightHours(int units);
void lightMins(int units);

void sound_alarm(void);
void stop_alarm(void);
void trigger_alarm(void);
void start_monitor(void);
void reset_sysTime (void);
void change_frequency(void);

// define constants
const char RTCAddr = 0x6f;
const char SEC = 0x00; // see register table in datasheet
const char MIN = 0x01;
const char HOUR = 0x02;
const char TIMEZONE = 2; // +02H00 (RSA)

// define pins
const int LEDS[] = {0,2,3}; // for the alarm
const int SECS = 1;
const int BTNS[] = {21,29,23,24}; // B0 - B3


#endif
