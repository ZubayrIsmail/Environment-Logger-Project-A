#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#define TRUE                (1==1)
#define FALSE               (!TRUE)
#define CHAN_CONFIG_SINGLE  8
#define CHAN_CONFIG_DIFF    0
#define PLAY_STOP_BUTTON  4
#define DISMISS_BUTTON 5
#define FREQ_BUTTON 6
long lastInterruptTime = 0; //Button Debounce
bool playing = FALSE;

static int myFd ;
 //Function to load drivers for SPI
void loadSpiDriver()
{
    if (system("gpio load spi") == -1)
    {
        fprintf (stderr, "Can't load the SPI driver: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 //Function to setup the SPI

void spiSetup (int spiChannel)
{
    if ((myFd = wiringPiSPISetup (spiChannel, 10000)) < 0)
    {
        fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 //Function that reads data from the ADC 
int myAnalogRead(int spiChannel,int channelConfig,int analogChannel)
{
    if(analogChannel<0 || analogChannel>7)
        return -1;
    unsigned char buffer[3] = {1}; // start bit
    buffer[1] = (channelConfig+analogChannel) << 4;
    wiringPiSPIDataRW(spiChannel, buffer, 3);
    return ( (buffer[1] & 3 ) << 8 ) + buffer[2]; // get last 10 bits
}

//convert ADC reading of channel 2 to temperature in celcius
int temp(int channel){
    float i = 1023,j=0.01;
    int temp_reading;
    temp_reading = (((channel*3.3)/i)-0.5)/j;
    //temp_reading = round(temp_reading);
    return temp_reading;
    
    }
 //Convert ADC reading of channel 3 to a voltage
 
 float humidity_voltage_reading(int channel){
     float i=1023;
     float voltage =(channel/i)*3.3;
     
     return voltage;
     }   
//Function that checks if the system is logging or not thenchanges the state     
void play_stop(void){
    
    //Debounce
    long interruptTime = millis();

    if (interruptTime - lastInterruptTime>200){
        // If playing, pause
        if (playing == true)
        {
            playing = false; // Pause
	    //printf("Stopped\n");
        }
        // Else, if paused, play
        else
        {
            playing = true; // Play
	   //printf("Play\n");
        }
    }
    lastInterruptTime = interruptTime;
}
   
int setup_gpio(void){
    //Setting up wiringpi and SPI
    int spiChannel=0;
    wiringPiSetup () ;
    spiSetup(spiChannel);
    
    //Setting up the push buttons
    pinMode(PLAY_STOP_BUTTON , INPUT);
    pinMode(FREQ_BUTTON, INPUT);
    pinMode(DISMISS_BUTTON, INPUT);
    pullUpDnControl(PLAY_STOP_BUTTON , PUD_UP);
    pullUpDnControl(FREQ_BUTTON, PUD_UP);
    pullUpDnControl(DISMISS_BUTTON, PUD_UP);
    
    
    //interrupts to Buttons
    wiringPiISR (PLAY_STOP_BUTTON , INT_EDGE_FALLING,  play_stop);
    //wiringPiISR (FREQ_BUTTON, INT_EDGE_FALLING,  freq);
    return 0;
    }
int main (int argc, char *argv [])
{
    int spiChannel=0;
    int loadSpi=FALSE;
    int channelConfig=CHAN_CONFIG_SINGLE;
    if(loadSpi==TRUE){
        loadSpiDriver();}

    setup_gpio();
    int delay_time = 2000;              //can be removed when RTC is implemented 
    
    
    
    
    while(TRUE){
        
        if(playing ==true){            //only play if button was pressed
            //read values from each spi channel
            int ldr_adc_ch1 = myAnalogRead(spiChannel,channelConfig,0);
            int temp_adc_ch2 = myAnalogRead(spiChannel,channelConfig,1);
            int humidity_adc_ch3 = myAnalogRead(spiChannel,channelConfig,2);
            
            float i = 1023;
            float dac_out = (ldr_adc_ch1/i)*humidity_voltage_reading(humidity_adc_ch3); //DAC output voltage
            float humidity_voltage = humidity_voltage_reading(humidity_adc_ch3);
            
            printf("---------------------------------------------------------------------\n");
            printf("RTC Time    Sys Timer   Humidity    Temp    Light   DAC Out     Alarm\n");
            printf("---------------------------------------------------------------------\n");
            printf("10:17:12    00:00:00       %.1f       %d      %d     %.1f",humidity_voltage, temp(temp_adc_ch2),ldr_adc_ch1, dac_out );
            
            delay(delay_time);
            }
    }

    close (myFd) ;
    return 0;
}

