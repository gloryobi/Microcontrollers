/*
 * File:   lab3.c
 * Author: tranx627
 *
 * Created on October 11, 2016, 4:43 PM
 */

#include <p24Fxxxx.h>

#include <xc.h>

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

void setup(void){
    /*bclr CLKDIV,#8 ;RCDIV0
    mov #0x9FFF,w0
    mov w0,AD1PCFG ;set pins RB2 to RB9 as DIGITAL
    mov     #3,w0
    nop
    mov     w0,TRISB
    nop
    bset    TRISA,#0                  ;PORTA, bit 0, will be an input (safety first!)
    bset    CNPU1,#2                  ;turns on internal Pull UP on CN01 (RA0)
    
    mov #0xFFFF,w0
    mov w0,PORTB ;set PORTB to 1111 1111 1111 1100*/
    CLKDIVbits.RCDIV0 = 8;
    AD1PCFG = 0x9fff;
    TRISB = 3;
    TRISA |= 0x1;
    CNPU1 |= 0x4;
    PORTB = 0xFFFF;
}

unsigned long int Random(unsigned long int seedx){
    unsigned long int seed;
    seed = 0x0005DEECE66D*seedx + 11;
    return seed;
}

void delay (void){
    int temp;
    int i, j ,k;
    for(i = 0; i < 5; i++){
        for(j = 0; j < 5; j++){
            for(k = 0; k < 5; k++){
                temp = 8000;
                while(temp != 0){
                    temp -= 1;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    unsigned long int seed;
    int temp;
    seed = 0x00000001330E;
    
    setup();
    
    while(1){
        if(PORTAbits.RA0 == 0){
            seed = Random(seed);
            temp = seed << 2;
            delay();
            PORTB = temp;
        }
    }
    return 0;
}
