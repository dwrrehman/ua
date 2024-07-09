/*
	202407081.133041: dwrr
	a program to run on the msp430fr2433 
	to communicate with the riscv computer
	reliably using only a gpio single pin. 
	because the pcie controller is unreliable.
*/

#include <msp430.h>				

static void set_data_0(void) {
	P2OUT &= ~BIT1;
}

static void set_data_1(void) {
	P2OUT |= BIT1;
}

static void direction_output(void) {
	P2DIR |= BIT1;
}

static void direction_input(void) {
	P2DIR &= ~BIT1;
}

static int get_data(void) {
	if (P2IN & BIT1) return 1; else return 0;
}

static int button_pressed(void) {
	if (P1IN & BIT2) return 0; else return 1;
}

static int sleep_1(void) {
	{ volatile unsigned int i; for (i = 1; i--;) {
		{ volatile unsigned int j; for (j = 1; j--;) { } }
	} }
}

// x is a unsigned byte value: 0 through 255.
static void transmit(int x) {
	direction_output();
	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1(); sleep_1(); sleep_1(); sleep_1();
	{int _; for (_ = 0; _ < 3; _++) {
		{int i; for (i = 0; i < 8; i++) {
			set_data_1();
			sleep_1();
			set_data_0();
			if ((x >> i) & 1) { sleep_1(); sleep_1(); sleep_1(); }
			sleep_1();
		}}
	}}
	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1();
	set_data_1();
	sleep_1();
	set_data_0();
	sleep_1();
	direction_input();
}

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	SFRRPCR |= SYSRSTUP;

	PMMCTL0_H = PMMPW_H;
	PMMCTL0_L &= ~SVSHE; //  disable voltage supervisor, to operate at exactly 1.80v 
	PMMCTL0_H = 0;

	// mov.b #0xA5, &FRCTL0_H
	// mov.b #0x10, &FRCTL0_L      ; increase NWAITS to 1, for 16mhz.
	// clr.b &FRCTL0_H
	// mov.b #0x3B, &CSCTL1_L      ; run at 16MHz using the DCO.

	CSCTL4 = 0x0203;
	CSCTL5 = 0x1006; 

	// mov.w #0x0203, &CSCTL4_L      ; set MCLK and ACLK to vlo.
	// mov.w #0x1007, &CSCTL5_L      ; set divider to be 128, using MCLK (vlo)

	SYSCFG0 = 0xA500;       // enable writing to all data/code FRAM.

	P1DIR = 0xFF;       // set all pins as an output, outputting 0.
	P1OUT = 0;
	P2DIR = 0xFF;
	P2OUT = 0;
	P3DIR = 0xFF;
	P3OUT = 0;

	P1DIR &= ~BIT2;       // 2.0 pin as button input
	P1REN |= BIT2;        // 2.0 input resistor enable
	P1OUT |= BIT2;        // 2.0 pullup resistor
	direction_output();
	PM5CTL0 &= ~LOCKLPM5;  // unlock gpio.

	int y = 0;
	while (1) {
		if (button_pressed()) transmit(0xAC);
		else {
			if (y == 0) { P2OUT ^= BIT2; y = 1; }
			else if (y == 1) { P2OUT ^= BIT3; y = 2; }
			else { P3OUT ^= BIT0; y = 0; }
		}
		sleep_1();
	}
}


