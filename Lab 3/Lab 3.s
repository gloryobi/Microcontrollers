    .equ __P24FJ64GA002,1
    .include "p24Fxxxx.inc"

#include "xc.inc"

        config __CONFIG2, POSCMOD_EC & I2C1SEL_SEC & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSECME & FNOSC_FRCPLL & SOSCSEL_LPSOSC & WUTSEL_FST & IESO_OFF
        config __CONFIG1, WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & COE_ON & BKBUG_ON & GWRP_ON & GCP_ON & JTAGEN_OFF

    .bss
seed: .space 2
seed1: .space 2
seed2: .space 2
tmp:    .space  2
count:  .space  2
    .text
    .global _main
 
_main:
    bclr CLKDIV,#8 ;RCDIV0
    mov #0x9FFF,w0
    mov w0,AD1PCFG ;set pins RB2 to RB9 as DIGITAL
    mov     #3,w0
    nop
    mov     w0,TRISB
    nop
    bset    TRISA,#0                  ;PORTA, bit 0, will be an input (safety first!)
    bset    CNPU1,#2                  ;turns on internal Pull UP on CN01 (RA0)
    
    mov #0xFFFF,w0
    mov w0,PORTB ;set PORTB to 1111 1111 1111 1100
    
initial: ;configures 3 16-bit seeds (16*3=48)

    mov #0x330E,w0
    mov w0,seed ;initial seed of 1 for random number gen.
    mov #0x0001,w0
    mov w0,seed1
    mov #0x0000,w0
    mov w0,seed2 ;set the other 2 seeds to zero

Random:
    
    btsc PORTA,#0 ;keep looping while PORTA reads 0
    bra Random ;if not, proceed to multiplying stage
multiply48: ;performs '48-bit' multiplication and randomizaton
			    ;by multiplying the seeds
;with parameter 0x0005DEECE66D
;and adding 11
;1st seed x 0xE66D (LSB)
;2nd seed x 0xDEEC
;3rd seed x 0x0005 (MSB)
    mov #0xE66D,w3;a0
    mov #0xDEEC, w4;a1
    mov #0x0005, w5;a2
    mov seed, w0
    mov w0, w12
    
    mul.uu w0, w3, w6
    mov w6, seed
    
    mul.uu w0, w4, w8
    mov seed1, w0
    mov w0, w13
    mul.uu w0, w3, w10
    add w7, w8, w6
    addc w6, w10, w6
    mov w6, seed1
    
    mov w12, w0
    mul.uu w0, w5, w6
    mov w13, w0
    mul.uu w0, w4, w8
    mov seed2, w0
    mul.uu w0, w3, w12
    add w6, w9, w6
    addc w6, w11, w6
    addc w6, w8, w6
    addc w6, w12, w6
    mov w6, seed2
    
    mov seed, w0
    add w0,#11,w0 ;add 11 to 1st seed
    mov w0,seed
    mov #5, w4
    
loop0:
    mov     #5,w1
    mov     w1,tmp
    mov #5, w2
loop1:
    dec     w2,w2
    nop
    bra     NZ,loop1
    dec     tmp
    bra     NZ,loop1
    mov     #0xffff,w3
loop2:
    dec     w3,w3
    bra     NZ,loop2
    nop
    
loop3:
    dec     w4,w4
    bra     NZ,loop0
    nop
    
output:
    com w0,w0 ;toggle WREG0
    sl w0,#2,w0 ;perform left shift by 2
    mov w0,PORTB ;move WREG0 to LATB for output
    bra Random
    .end