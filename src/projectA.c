
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int Vout, sample;
int hours, mins, secs;
int alarm_hours, alarm_mins;
int sys_hours, sys_mins, flag;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH,MM,SS;
int hoursList[8], minsList[7];

void initGPIO(void){
	
	wiringPiSetup(); //This is the default mode. 
	
	cleanUp(); //resets pins to avoid writing to the same hardware multiple times

	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

	// BUTTONS AND LEDS
	
	    pinMode(LEDS[0], OUTPUT);
	    pinMode(LEDS[1], OUTPUT);
	    pinMode(LEDS[2], OUTPUT);
	    //printf("LEDs done \n");
	
		pinMode(BTNS[0], INPUT);
		pinMode(BTNS[1], INPUT);
		pinMode(BTNS[2], INPUT);
		pinMode(BTNS[3], INPUT);
		//pullUpDnControl(BTNS[j], PUD_UP);
		//printf("BUttons done");
	

	//printf("both done");


}

void cleanUp (void){

	for(int i = 0; i<sizeof(LEDS); i++){
		digitalWrite(LEDS[i], 0);
	
	}
}

int main(void){
	
	initGPIO();
	alarm_mins = 59;
	alarm_hours = 0;
	sys_hours = sys_mins = flag = 0;
	sample = 1;

 // Fetch time from the internet
	wiringPiI2CWriteReg8(RTC, SEC, 0x80 ); // activates the RTC to starts counting

	//our interrupts
	if (wiringPiISR (BTNS[3], INT_EDGE_FALLING, &stop_alarm) < 0){ //increase hours
		printf("Unable to do stuff" );
		return 1;
	}

	if (wiringPiISR (BTNS[1], INT_EDGE_FALLING, &start_monitor) < 0){ //increase hours
		printf("Unable to do stuff" );
		return 1;
	}

	if (wiringPiISR (BTNS[2], INT_EDGE_FALLING, &reset_sysTime) < 0){ //increase hours
		printf("Unable to do stuff" );
		return 1;
	}


	if (wiringPiISR (BTNS[0], INT_EDGE_FALLING, &change_frequency) < 0){ //increase hours
		printf("Unable to do stuff" );
		return 1;
	}


	for (;;){
		//Fetch the time from the RTC
		hours = hexCompensation(wiringPiI2CReadReg8(RTC, HOUR));
		mins = hexCompensation(wiringPiI2CReadReg8 (RTC, MIN));
		secs = hexCompensation(wiringPiI2CReadReg8 (RTC, SECS)-0x80); //removes the extra bit that enables the seconds register
		
		
		// Print out the time we have stored on our RTC
		printf("TIME\n");
		printf("The current time is: %dh:%dm\n",hours, mins);

		if (Vout < 0.65){

			sound_alarm();
		}

		if (Vout > 2.65){

			sound_alarm();
		}



		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
	}
	return 0;
}

/*
 * Function that converts the binary variable to an array of separate bits
 * 
 */
void int_to_bin_digit(unsigned int in, int count, int* out){
    /* assert: count <= sizeof(int)*CHAR_BIT */
    unsigned int mask = 1U << (count-1);
    int i;
    for (i = 0; i < count; i++) {
        out[i] = (in & mask) ? 1 : 0;
        in <<= 1;
    }
}
int hexCompensation(int units){
	/*Convert HEX or BCD value to DEC where 0x45 == 0d45 
	  This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
	  perform operations which work in base10 and not base16 (incorrect logic) 
	*/
	int unitsU = units%0x10;

	if (units >= 0x50){
		units = 50 + unitsU;
	}
	else if (units >= 0x40){
		units = 40 + unitsU;
	}
	else if (units >= 0x30){
		units = 30 + unitsU;
	}
	else if (units >= 0x20){
		units = 20 + unitsU;
	}
	else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}

/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units){
	int unitsU = units%10;

	if (units >= 50){
		units = 0x50 + unitsU;
	}
	else if (units >= 40){
		units = 0x40 + unitsU;
	}
	else if (units >= 30){
		units = 0x30 + unitsU;
	}
	else if (units >= 20){
		units = 0x20 + unitsU;
	}
	else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}

void sound_alarm(void){
	//debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){

		if (alarm_hours == hours){

			printf("mins is%d\n", alarm_mins );

			if ((alarm_mins - mins) > 3){
				
				//turn on leds
				
					digitalWrite (LEDS[0],1);
					digitalWrite (LEDS[1],1);
					digitalWrite (LEDS[2],1);
					printf("Alarm Enabled = %d\n", (alarm_mins - mins));

			}
		}
		alarm_hours = hours;
		alarm_mins = mins;		//update time

	}
	lastInterruptTime = interruptTime;
}

void stop_alarm(void){

	//debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){

		printf("Alarm disabled\n");

	}
	lastInterruptTime = interruptTime;
}

void start_monitor(void){

	//debounce
	long interruptTime = millis();
	if (interruptTime - lastInterruptTime>200){

		switch(flag){

			case 0:
				printf("Monitoring");
				sys_hours = hours;
				sys_mins = mins;
				flag = 1;
				break;
			
			case 1:
				printf("Monitoring Paused");
				flag = 3;
				break;

			case 3:
				printf("Monitoring");
				flag = 1;
				break;
		}

	}
	lastInterruptTime = interruptTime;
}


void reset_sysTime (void){
	//debounce
	long interruptTime = millis();
	if (interruptTime - lastInterruptTime>200){

		sys_hours = 0;
		sys_mins = 0;
		printf(" sys_mins %d %d \n",sys_mins, sys_hours);

	}
	lastInterruptTime = interruptTime;
}

void change_frequency(void){

	//debounce
	long interruptTime = millis();
	if (interruptTime - lastInterruptTime>200){

		switch(sample){

			case 1:
				printf("works\n");
				break;

			case 2:

			break;

			case 5:

				break;

			default:


		}

	}
	lastInterruptTime = interruptTime;
}
