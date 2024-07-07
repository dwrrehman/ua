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


void main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	SFRRPCR |= SYSRSTUP;

	//mov.b #0xA5, &FRCTL0_H
	//mov.b #0x10, &FRCTL0_L      ; increase NWAITS to 1, for 16mhz.
	//clr.b &FRCTL0_H
	//mov.b #0x3B, &CSCTL1_L      ; run at 16MHz using the DCO.

	SYSCFG0 = 0xA500;       // enable writing to all data/code FRAM.

	P1DIR = 0xFF;       // set all pins as an output, outputting 0.
	P1OUT = 0;
	P2DIR = 0xFF;
	P2OUT = 0;
	P3DIR = 0xFF;
	P3OUT = 0;

	P1DIR &= ~BIT2;        // 2.0 pin as button input
	P1REN |= BIT2;        // 2.0 input resistor enable
	P1OUT |= BIT2;        // 2.0 pullup resistor
	direction_output();
	PM5CTL0 &= ~LOCKLPM5;  // unlock gpio.
	int x = 0;
	int y = 0;
	while (1) {
		if (P1IN & BIT2) {
			if (y == 0) { P2OUT ^= BIT2; y = 1; }
			else if (y == 1) { P2OUT ^= BIT3; y = 2; }
			else { P3OUT ^= BIT0; y = 0; }
		} else {
			if (x) { set_data_1(); x = 0; }
			else { set_data_0(); x = 1; }
		}
		{ volatile unsigned int i; for (i = 300; i--;) {
			{ volatile unsigned int j; for (j = 300; j--;) { } }
		} }
	}
}


