#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified                                                 @ Specify unified assembly syntax
.cpu    cortex-m0plus                                           @ Specify CPU type is Cortex M0+
.thumb                                                          @ Specify thumb assembly for RP2040
.global setup_gpio_isr
.align 4                                                        @ Specify code alignment

.equ        GPIO_BTN_IN_LOW, 0x00400000                         @ Define a value representing the button state when it is initially pressed
.equ        GPIO_BTN_IN_HIGH, 0x00800000                        @ Define a value representing the button state when a button press is terminated
.equ        GPIO_BTN_IN, 21                                     @ Define a variable for the PI pin 21 which will be used

.equ        GPIO_ISR_OFFSET, 0x74 

@ Set up the gpio_isr to correctly react to interrupts
setup_gpio_isr:
    push    {lr}

    ldr	    r2, =PPB_BASE                       @ Load r2 with base address
	ldr	    r1, =M0PLUS_VTOR_OFFSET             @ Load r1 with offset for vector table
	add	    r2, r1                              @ Add r2 and r1 to get the vector table address
	ldr	    r1, [r2]                            @ Load r1 with contents at address stored in r2
	movs    r2, #GPIO_ISR_OFFSET                @ Store gpio offset (Vector table entry 29)
    add	    r2, r1                              @ Add offset to vector table
	ldr	    r0, =gpio_isr                       @ Load r0 with address for the gpio interrupt handler
	str	    r0, [r2]                            @ Store address r0 at the gpio entry point

	movs 	r0, #1                              
    lsls    r0, #13                             @ Logical Shift r0 13, IRQ13 = gpio
	ldr	    r2, =PPB_BASE                       @ Load r2 with base address
	ldr	    r1, =M0PLUS_NVIC_ICPR_OFFSET        @ Load r1 with offset for NVIC interrupt clear pending register
	add	    r1, r2                              @ Add r1 and r2, Offset base in r2 with offset stored in r1
	str	    r0, [r1]                            @ Store a 1 to ICPR to clear the interrupt
	ldr	    r1, =M0PLUS_NVIC_ISER_OFFSET        @ Load r1 with offset for NVIC interrupt set enable register
	add	    r1, r2                              @ Add r1 and r2, offset base in r2 with offset stored in r1
	str	    r0, [r1]                            @ Store a 1 to ISER to enable interrupt
	
	POP     {pc}                                  @ Return 

@ Reaction to a button interrupt
.thumb_func
gpio_isr:
    push    {lr}

    bl      watchdog_update                     @ Update watchdog

    ldr     r2, =IO_BANK0_BASE                  @ Load r2 with base for IO 
    ldr     r1, =IO_BANK0_PROC0_INTS2_OFFSET    @ Load r1 with base offset for GPIO interrupt statue (#2)
    add     r2, r1                              @ Add offset to base and store value in r2

    ldr     r0, [r2]                            @ Load GPIO interrupt status into r0
    ldr     r1, =GPIO_BTN_IN_LOW                @ Load r1 with mask for GP21 "Enable Button"
    ands    r0, r0, r1                          @ AND the mask in r1 and the status in r0
    cmp     r0, r1                              @ Check if result stored in r0 is the same as the mask in r1
    beq     gpio_low                            @ Falling Edge interrupt

    b       gpio_high                           @ Otherwise Rising Edge

@ Enable timer here, count how long dot was for
gpio_low:
    bl      get_t1                              @ Obtain a time value when the button is initially pressed
    ldr     r0, =GPIO_BTN_IN_LOW                @ Load this value into r0
    b       clear_gpio                          @ Clear this interrupt state

@ Check timer length, if over Xms then dash otherwise dot
gpio_high:
    bl        get_t2                            @ Obtain a time value when the button press is terminated
    bl        determine                         @ Calculate the total length of timeof the button press
next:
    ldr     r0, =GPIO_BTN_IN_HIGH               @ Store this value into the relevant address in memory
    b       clear_gpio                          @ Clear this interrupt state

@ Clear gpio interrupt
clear_gpio:
    ldr     r2, =IO_BANK0_BASE                  @ Load r2 with GPIO base address
    ldr     r1, =IO_BANK0_INTR2_OFFSET          @ Load r1 with GPIO interrupts 2 register
    add     r2, r1                              @ Offset base address in r2 by value in r1
	str	    r0, [r2]                            @ Store clear bit stored in r0 to address
    POP     {pc}

@ Set data alignment
.data
    .align 4
