#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <time.h>
 
#define TRUE                (1==1)
#define FALSE               (!TRUE)
#define CHAN_CONFIG_SINGLE  8
#define CHAN_CONFIG_DIFF    0
 
static int myFd ;
 
void loadSpiDriver()
{
    if (system("gpio load spi") == -1)
    {
        fprintf (stderr, "Can't load the SPI driver: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 
void spiSetup (int spiChannel)
{
    if ((myFd = wiringPiSPISetup (spiChannel, 10000)) < 0)
    {
        fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 
int myAnalogRead(int spiChannel,int channelConfig,int analogChannel)
{
    if(analogChannel<0 || analogChannel>7)
        return -1;
    unsigned char buffer[3] = {1}; // start bit
    buffer[1] = (channelConfig+analogChannel) << 4;
    wiringPiSPIDataRW(spiChannel, buffer, 3);
    return ( (buffer[1] & 3 ) << 8 ) + buffer[2]; // get last 10 bits
}



int main (int argc, char *argv [])
{
    int loadSpi=FALSE;
    int spiChannel=0;
    int channelConfig=CHAN_CONFIG_SINGLE;

    if(loadSpi==TRUE)
        loadSpiDriver();
    wiringPiSetup () ;
    spiSetup(spiChannel);
    
    int delay_time = 5000;
    while(TRUE){
        int ldr_adc_ch1 = myAnalogRead(spiChannel,channelConfig,0);
        int temp_adc_ch2 = myAnalogRead(spiChannel,channelConfig,1);
        int humidity_adc_ch3 = myAnalogRead(spiChannel,channelConfig,2);
        
        printf("MCP3008(CE%d): LDR_ADC = %d\n",spiChannel,ldr_adc_ch1);
        printf("MCP3008(CE%d): temp_ADC = %d\n",spiChannel,temp_adc_ch2);
        printf("MCP3008(CE%d): humidity_ADC = %d\n",spiChannel,humidity_adc_ch3);
        delay(delay_time);
    }

    close (myFd) ;
    return 0;
}

