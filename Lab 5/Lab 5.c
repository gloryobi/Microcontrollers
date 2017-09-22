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

//Global Variables
#define NOKEY 255 // value of input corresponding to no key being pressed
#define CLEAR_ALL_DIG_AND_SEGS_BITS_MASK    0b1111000000000011   // to be ANDED
#define TURN_ON_DIG1_MASK           0b0000010000000000  // to be ORed after the 
#define TURN_ON_DIG2_MASK           0b0000100000000000  //    CLEAR_ALL... mask

// Patterns - 7 Segment Display Encoding
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
   0b0000000000100100,   // '9'
   0b0000000001000100,   // 'A'
   0b0000001100000100,   // 'b'
   0b0000000110001100,   // 'C'
   0b0000001000010100,   // 'd'
   0b0000000110000100,   // 'E'
   0b0000000111000100,   // 'F'
   0b0000001111111100    // all off
};

// Display Variables
unsigned int digits[2] = {0x000C, 0x000C}; // holds display encoding of values to be displayed

// Keypad variables
unsigned char cbuffer[2]; // holds the digits to be displayed

// KBMASK1 is used to mask off the bits in PORTB we don't use (RB12-RB0)
#define KBMASK1 0x0fff
unsigned int keyRowEnPattern[4] = {0xe000, 0xd000, 0xb000, 0x7000};

void multiplexDisplay(void)
{
   static unsigned int d = 0;

   LATB &= CLEAR_ALL_DIG_AND_SEGS_BITS_MASK;

   switch (d)
   {
   case 0:
       LATB |= digits[0] | TURN_ON_DIG1_MASK;
       break;
   case 1:
       LATB |= digits[1] | TURN_ON_DIG2_MASK;
       break;
   }

   d = 1 - d;
}

void updupdateDigitsArray_7segPatternsDecimal()
{
   digits[0] = patterns[cbuffer[1]]; // sets the left digit
   digits[1] = patterns[cbuffer[0]]; // sets the right digit
}

unsigned char lookupTable[16] = {1, 2, 3, 10 , 4, 5, 6, 11, 7, 8, 9, 12, 12,
   0, 14, 13}; // assuming the normal encoding.  To change it, just switch
                // around the elements to make things turn out like you want.
// Scan 
unsigned char scan(unsigned short int row)
{
   static unsigned char LUT[] = {NOKEY,NOKEY,NOKEY,NOKEY,NOKEY,NOKEY,NOKEY,0,
       NOKEY,NOKEY,NOKEY,1,NOKEY,2,3,NOKEY};
   unsigned char key;

   key = PORTA & 15;
   key = LUT[key];
   if (key == NOKEY) return key;
   key += 4*row;
   key = lookupTable[key];

   return  key;
}

void put(unsigned char x)
{
   cbuffer[1] = cbuffer[0];
   cbuffer[0] = x;
}

void waitOneMillisecond(void)
{
   static unsigned short int row = 0;
   static unsigned char lastKey[] = {NOKEY,NOKEY,NOKEY,NOKEY};
   static unsigned short int debounce  = 0;
   unsigned char key;

   while (!IFS0bits.T1IF) ;
   IFS0bits.T1IF = 0;
   if (debounce)
   {
       debounce--;
       return;
   }
   key = scan(row);
   if (key != lastKey[row])
   {
       lastKey[row] = key;
       if (key != NOKEY)
       {
           put(key);
           updupdateDigitsArray_7segPatternsDecimal();
       }
       debounce = 40;
   }
   row++; // move to next row
   row &= 3;
   LATB = LATB & KBMASK1;
   LATB |= keyRowEnPattern[row]; // sets the next row to be scanned
}

void setup(void)
{
   CLKDIVbits.RCDIV = 0;
   AD1PCFG = 0x9fff;  // For digital I/O
   TRISAbits.TRISA0 = 1; // input from keypad
   TRISAbits.TRISA1 = 1;
   TRISAbits.TRISA2 = 1;
   TRISAbits.TRISA3 = 1;
   CNPU1bits.CN2PUE = 1; // set pull up resistors
   CNPU1bits.CN3PUE = 1;
   CNPU2bits.CN30PUE = 1;
   CNPU2bits.CN29PUE = 1;

   LATBbits.LATB12 = 1; // output to keypad rows
   LATBbits.LATB13 = 1;
   LATBbits.LATB14 = 1;
   LATBbits.LATB15 = 1;
   TRISB = 0;

   LATB |= 0b0000001111111100;   // turn all 7-seg LEDs off
   LATB &= 0b1100001111111111;   // turn off all displays
   
// Timer setup
   T1CON = 0;
   T1CONbits.TCKPS = 0;
   PR1 = 15999; // 1 ms delay
   TMR1 = 0;
   IFS0bits.T1IF = 0;
   T1CONbits.TON = 1;
}

int main(int argc, char *argv[])
{
   setup();
   
   while (1)
   {
       multiplexDisplay();
       waitOneMillisecond();
   }
   return 0; 
}