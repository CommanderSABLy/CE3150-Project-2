/*
 * Project2.c
 *
 * Created: 5/9/2020 10:28:50 PM
 * Author : Saul Lynn, Shane McEnaney, Pamela Petterchak, Adam Sawyer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h> // bit_is_clear(PIND, PD2)

ISR (TIMER0_OVF_vect);

#define TOT_ITERATIONS 25
#define TOT_HALF_PER 10

unsigned char tenth = 0;
unsigned char ones = 0;
unsigned char iterations;
unsigned char halfPer;

int mode = 0;

int main() {
	DDRD = 0xFF;
	PORTD = 0xFF;
	DDRA = 0x00; // make PA input
	PORTA = 0xFF; // enable pull up on PA
	
	TCNT0 = -125;
	TCCR0A = 0x00;
	TCCR0B = 0x04;
	TIMSK0 = 0x01;
	
	sei();
	
	iterations = 0;
	halfPer = 0;
	
	while (1) {
		if (bit_is_clear(PINA, PA0)) {
			mode = 1;
		} else if (bit_is_clear(PINA, PA1)) {
			mode = 0;
		} else if (bit_is_clear(PINA, PA2)) {
			mode = -1;
		}
	}
}

ISR (TIMER0_OVF_vect) {
	iterations += 1;
	if (iterations < TOT_ITERATIONS) {
		TCNT0 = -125;
	} else if (halfPer < TOT_HALF_PER) {
		PORTD ^= (ones*16 + tenth);
		if (mode == 1) {
			tenth++;
			if (tenth == 10) {
				tenth = 0;
				ones++;
				if (ones == 10) {
					ones = 0;
				}
			}
		} else if (mode == 2) {
			tenth--;
			if (tenth == -1) {
				tenth = 9;
				ones--;
				if (ones == -1) {
					ones = 9;
				}
			}
		} else if (mode == -1) {
			tenth = 0;
			ones = 0;
		}
		PORTD ^= (ones*16 + tenth);
		iterations = 0;
		TCNT0 = -125;
		halfPer += 1;
	} else {
		iterations = 0;
		halfPer = 0;
		TCNT0 = -125;
	}
}


