#include <p24Fxxxx.h>
// #include <PPS.h>  // older versions of the XC16 compiler

#include <xc.h>

#include <stdlib.h>
#include <math.h>

// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
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

volatile unsigned char buffer[32];
volatile unsigned char front = 0;
volatile unsigned char back = 0;

// This code is for interfacing with the MCP4821 DAC unit, which has an SPI interface.
// We need to send 16-bit SPI output, where the four most significant bits are the commands
// determining the gain and whether to shut down the DAC unit, followed by 12 bits of data.
// The SPI clock speed of 16MHz is not too fast (the DAC manual says 20MHz is the max), and if we
// want slower transmission, we can choose a PRE of 1:2 in the SPI clock.
// We setup T2 to give us 22.7us for each analog voltage. The 22.7us is safely longer than the
// 8 * SPI_CLOCK_PERIOD, which is 0.5us with a primary and secondary SPI clock pre set to 1.


#define DACMASK 0x1FFF
#define DACBITS 0x1000

volatile unsigned short int sample = DACBITS;
volatile float time = 0;

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    PORTBbits.RB7 = 0;  //Drive Chip Select to 0
    
    ///*  Lab Sine Wave
    sample &= DACMASK;
    sample |= DACBITS;
    IFS0bits.T2IF = 0;
    SPI1BUF = sample;
    
    sample = (( sin(6.28 * time) + sin(15.71 * time) ) * 512 ) + 1024; //Precautions to make sure sample doesn't overflow or go negative
    
    time += .01; //Frequency Controller
     //*/
    if(time == 1.0)
	  time = 0;
    
    PORTBbits.RB7 = 1;  //Drive Chip Select High
}


void __attribute__((__interrupt__,__auto_psv__)) _SPI1Interrupt(void)
{
    _SPI1IF = 0;
    /*
    PORTBbits.RB6 = 0;  // SPI transaction complete, give a low pulse on LDAC'
        // to load the value from the input latch register to the DAC register.
    // The DAC datasheet says that the pulse has to remain low for at least
    // 100ns (p6 of the manual, the second table)
    asm("nop");
    asm("nop");
    PORTBbits.RB6 = 1;
    */
}


void setup(void)
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9fff;  // For digital I/O.  If you want to use analog, you'll
                       // need to change this.
    TRISB = 3;    // make all PORTBbits inputs initially

    // add your configuration commands below
    PORTB = 0xffff;
    //TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB7 = 0; // this will be the DAC /LDAC. I assume /CS is hardwired to GND

    T2CON = 0;
    PR2 = 362; // Period of step will be 363 Tcy = 22.7 us
    T2CONbits.TON = 1;

    // Peripheral Pin Select 
    // Ver 1.25 or later
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR2bits.RP5R = 7;   //RB5->SPI1:SDO1; See Table 10-3 on P109 of the datasheet
    RPOR3bits.RP6R = 8;   //RB4->SPI1:SCK1OUT;
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS


    SPI1CON1 = 0;
    SPI1CON1bits.MSTEN = 1;  // master mode
    SPI1CON1bits.MODE16 = 1; // sixteen bits
    SPI1CON1bits.CKE = 1;
    SPI1CON1bits.CKP = 0;
    SPI1CON1bits.SPRE = 0b110; // secondary prescaler = 2
    SPI1CON1bits.PPRE = 0b11;  // primary prescaler = 1;
    // SPI1CON1bits.PPRE = 0b01;  // primary prescaler = 16;
    SPI1CON2 = 0;
    SPI1STAT = 0;
    SPI1STATbits.SISEL = 0b101; // IF set when last bit is shifted out
                                // That means the SPI xfer is complete.
    SPI1STATbits.SPIEN = 1;


    IFS0bits.T2IF = 0;
    TMR2 = 0;

    IPC1bits.T2IP = 5;
    IEC0bits.T2IE = 1;
    _SPI1IF = 0;
    _SPI1IE = 1;
}

int main(int argc, char *argv[])
{
    setup();

    while (1)
    {
    }

    return 0; // never reached (we hope)
}
