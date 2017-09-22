#include <p24Fxxxx.h>
#include "xc.h"
#include <string.h>
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

static int ASCII[14] = {'0','1','2','3','4','5','6','7','8','9','.','V','m',13};
volatile int buffer[16] = {'\r','1','.','3','4','2','5','V',' ','1','2','3','.','1','m','V'};
volatile double voltage[512];
volatile int count = 16;
volatile int index = 0;
volatile double temp = 0;
int i;

void __attribute__((__interrupt__,__auto_psv__)) _U1TXInterrupt(void)
{
    IFS0bits.U1TXIF = 0;
    U1TXREG = buffer[count];
    count++;
    count %= 8;
}
void __attribute__((__interrupt__)) _ADC1Interrupt(void) {
    _AD1IF = 0;
    voltage[index] = ADC1BUF0 * 0.0032265625;//each integer is worth 0.00322265625
    temp = voltage[index];
    buffer[1] = temp + 48;
    for(i=3; i<7;i++){
        buffer[i] = (int)(temp * pow(10,i-2))%10 + 48;
    }
    index++;
    index %= 512;
}
void setup(void)
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0xFFFE; // AN0 is the only analog pin we are using
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS = 0;
    AD1CON1bits.ASAM = 1; // auto-sampling?
    AD1CON1bits.SSRC = 0b111; // auto-conversion
    AD1CON3bits.SAMC = 10; // auto-sample time = X * TAD (what is X?)
    AD1CON3bits.ADCS = 2; // clock = Y * Tcy (what is Y?)
    //Configure A/D interrupt
    _AD1IF = 0; // clear flag
    _AD1IE = 1; // enable interrupt
    AD1CON1bits.ADON = 1;
    _TRISB3 = 0; // U1TX output
    U1MODE = 0;
    U1BRG = 34; // 115200 baud,
    U1MODEbits.BRGH = 1;
    U1MODEbits.UEN = 0;
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    U1STAbits.UTXISEL1 = 1;
    U1STAbits.UTXISEL0 = 0;     //If not working check UTXISEL.
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    _RP3R = 0x0003;   //RB6->UART1:U1TX; See Table 10-3 on P109 of the datasheet
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS
    IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 1;
}
void putty(const unsigned char ch)
{
    while (!IFS0bits.U1TXIF) ;
    IFS0bits.U1TXIF = 0;
    U1TXREG = ch;
}
int main(void) {
    setup();
    U1TXREG = ' ';
    while(1);
    return 0;
}
