        .equ __P24FJ64GA002,1
        .include "p24Fxxxx.inc"

#include "xc.inc"

        config __CONFIG2, POSCMOD_EC & I2C1SEL_SEC & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSECME & FNOSC_FRCPLL & SOSCSEL_LPSOSC & WUTSEL_FST & IESO_OFF
        config __CONFIG1, WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & COE_ON & BKBUG_ON & GWRP_ON & GCP_ON & JTAGEN_OFF

    .bss
tmp:    .space  2
count:  .space  2
    .text
    .global _main
_main:
    bclr    CLKDIV,#8                 ;RCDIV0

    mov     #0x9e3f,w0
    mov     w0,AD1PCFG
    nop
    mov     #0xffff,w0
    mov     w0,PORTB
    nop
    mov     #3,w0
    nop
    mov     w0,TRISB
    nop
    clr     w1
    clr     count
loop0:
    mov     #6,w0
    mov     w0,tmp
loop1:
    dec     w1,w1
    nop
    bra     NZ,loop1
    dec     tmp
    bra     NZ,loop1
    mov     #9039,w3
loop2:
    dec     w3,w3
    bra     NZ,loop2
    nop
    inc     count
    mov     count,w2
    sl      w2,#2,w2
    mov     w2,PORTB
    bra     loop0
    .end
