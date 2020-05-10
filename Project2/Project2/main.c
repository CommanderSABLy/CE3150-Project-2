/*
 * Project2.c
 *
 * Created: 5/9/2020 10:28:50 PM
 * Author : Saul Lynn, Shane McEnaney, Pamela Petterchak, Adam Sawyer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

ISR (TIMER0_OVF_vect);

#define TOT_ITERATIONS 25
#define TOT_HALF_PER 10

unsigned char tenth = 0;
unsigned char iterations;
unsigned char halfPer;

int main() {
	DDRD = 0xFF;
	PORTD = 0xFF;
	
	TCNT0 = -125;
	TCCR0A = 0x00;
	TCCR0B = 0x04;
	TIMSK0 = 0x01;
	
	sei();
	
	iterations = 0;
	halfPer = 0;
	
	while (1) {
		// run
	}
}

ISR (TIMER0_OVF_vect) {
	iterations += 1;
	if (iterations < TOT_ITERATIONS) {
		TCNT0 = -125;
		} else if (halfPer < TOT_HALF_PER) {
		PORTD ^= tenth;
		tenth++;
		if (tenth == 10) {
			tenth = 0;
		}
		PORTD ^= tenth;
		iterations = 0;
		TCNT0 = -125;
		halfPer += 1;
		} else {
		iterations = 0;
		halfPer = 0;
		TCNT0 = -125;
	}
}


