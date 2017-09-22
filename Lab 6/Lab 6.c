#define FCL 1600000L
#define FCY 16000000L
#include <p24Fxxxx.h>
#include <stdlib.h>
#include <libpic30.h>
#include "xc.h"

#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSECME           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config SOSCSEL = SOSC           // Sec Oscillator Select (Default Secondary Oscillator (SOSC))
#pragma config WUTSEL = LEG             // Wake-up timer Select (Legacy Wake-up Timer)
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

const unsigned int patterns[] = {
    0b0000000000001100,   // '0'
    0b0000001001111100,   // '1'
    0b0000000010010100,   // '2'
    0b0000000000110100,   // '3'
    0b0000001001100100,   // '4'
    0b0000000100100100,   // '5'
    0b0000000100000100,   // '6'
    0b0000000001111100,   // '7'
    0b0000000000000100,   // '8'
    0b0000000001000100,   // '9'
    0b0000001111111100,   // all off
    0b0000001001110100,   // '+'
    0b0000001111110100    // '-'
};


#define CLEAR_ALL_DIG_AND_SEGS_BITS_MASK    0b1111000000000011   // to be ANDED
#define TURN_ON_DIG1_MASK           0b0000010000000011  // to be ORed after the 
#define TURN_ON_DIG2_MASK           0b0000100000000011  //    CLEAR_ALL... mask
unsigned char cbuffer[2] = {10,10};
unsigned short int digits[2] = {0x03Fc, 0x03Fc};
volatile long int overflow;
volatile int LEDF = 1;
long int time;
int blinktime=0;

void __attribute__((__interrupt__,__auto_psv__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    overflow++;
}

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{

    static int total = 0;
    IFS0bits.T2IF = 0;
    if(total < time)
        total++;
    else{
            total = 0;
            LEDF = 1 - LEDF;
        }
}

void update(long int time1, long int time2)
{
    long int dif=time1-time2; 
    cbuffer[1]= 0;
    cbuffer[0]= 0;
    if(dif<(-90*FCL))
    {
        cbuffer[1]= 12;
        cbuffer[0]= 12;
        LEDF=1;
    }
    else if(dif<0)
    {
        cbuffer[1]=12;
        cbuffer[0]=(-1*dif)/(FCL*10);
        LEDF=0;
    }
    else if (dif<(90*FCL))
    {
        cbuffer[1]=10;
        cbuffer[0]=dif/(FCL*10);
        LEDF=0;
    }
    else
    {
        cbuffer[1]=11;
        cbuffer[0]=12;
        LEDF=1;
    }
    
   
    digits[1] = patterns[cbuffer[1]];
    digits[0] = patterns[cbuffer[0]];
}
void multiplexDisplay(void)
{
    static unsigned int digit = 0;

    LATB &= CLEAR_ALL_DIG_AND_SEGS_BITS_MASK;
    
    
    switch (digit)
    {
    case 0:
        LATB |= digits[1] | TURN_ON_DIG1_MASK;
        break;
    case 1:
        LATB |= digits[0] | TURN_ON_DIG2_MASK;
        break;
    }
    digit=1-digit;
}

void blink(void)
{   
   
       if(LEDF)
        {   if(blinktime>500)
                {
                    //LATB |= 0b0000111111111100;
                    LATB   &= 0b1100001111111111; 
                }
            else
                {
                    multiplexDisplay();
                }
    
            blinktime = blinktime +1;
            blinktime = blinktime%1000;
        }
        else
            multiplexDisplay();

        __delay_ms(1);
        
    
    return;
}


void RandomGen(void)
{
    
    time = rand(); // where random returns a pseudorandom number from 0 to 65535 t = t % 9000; // gives a number from 0 to 8999 t += 1000; // now t is a number from 1000 to 9999.
    time = time % 9000; // gives a number from 0 to 8999 
    time += 1000; // now t is a number from 1000 to 9999.
}



void setup(void)
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9fff;  // For digital I/O.  If you want to use analog, you'll
                       // need to change this.

    // add your configuration commands below
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    CNPU1bits.CN2PUE = 1;
    
    LATB |= 0b0000111111111100;   // turn all 7-seg LEDs off
    LATB &= 0b1100001111111111;   // turn off all displays
    TRISB = 3;
  

    //event timer
    T1CON = 0;
    PR1 = 15999;
    TMR1 = 0;
    _T1IE= 1;
    
    //Random # Number Generator Timer
    PR2 = 15999;
    T2CON=0;
    T2CONbits.TON=1;
    TMR2=0;
    
    _RB15=1; //LED Off
    
    
}

int main(void) {
    long int time1,time2;
    setup();
    srand(0);
    while(PORTAbits.RA0 == 1);
        __delay_ms(20);
    while(PORTAbits.RA0 == 0);
        __delay_ms(20);
        
    while (1)
    {
        overflow = 0;
        RandomGen();
        
        TMR1=0;
        TMR2=0;
        _T2IE=1;
        while(LEDF==1);
        _T2IE=0;
        _RB15=0;
        T1CONbits.TON=1;
        while(PORTAbits.RA0 == 1);
        T1CONbits.TON=0;
        __delay_ms(20);
        while(PORTAbits.RA0 == 0);
        __delay_ms(20);
        time1= TMR1 + overflow*FCL;
        
        RandomGen();
        
        overflow = 0;
        TMR1=0;
        TMR2=0;
        _T2IE=1;
        while(LEDF==0);
        _T2IE=0;
        _RB15=1;
        T1CONbits.TON=1;
        while(PORTAbits.RA0 == 1);
        T1CONbits.TON=0;
        __delay_ms(20);
        while(PORTAbits.RA0 == 0);
        __delay_ms(20);
        time2= TMR1 + overflow*FCL;
        
        update(time1,time2);
        time1=0;
        time2=0;
        
       
        while(PORTAbits.RA0 == 1)
        {
            blink();
        }
        LEDF = 1;
        blinktime=0;
       // LATB |= 0b0000111111111100;   // turn all 7-seg LEDs off
        LATB &= 0b1100001111111111;   // turn off all displays
        
    }
    return 0;
}

