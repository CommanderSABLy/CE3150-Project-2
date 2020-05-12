/*
 * Project2.c
 *
 * Created: 5/9/2020 10:28:50 PM
 * Author : Saul Lynn, Shane McEnaney, Pamela Petterchak, Adam Sawyer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h> // bit_is_clear(PIND, PD2)

void timer();
void sound(int mode);
void alarm();

void USART_Init(unsigned long BR);
unsigned char USART_RxChar();
void USART_TxChar(char data);

ISR (TIMER0_OVF_vect);
ISR (USART2_UDRE_vect);
ISR (USART2_RX_vect);

#define TOT_ITERATIONS 25
#define TOT_HALF_PER 10
#define BAUDRATE 9600
#define F_CPU 16000000UL

unsigned char tenth = 0;
unsigned char ones = 0;
unsigned int total_time = 0;
unsigned char iterations;
unsigned char halfPer;

int mode = 0;
unsigned char message = '\0';

int main() {
	DDRD = 0xFF; // make output
	PORTD = 0xFF; // turn off active high LEDs
	DDRE = 0xBB; // make port 2 and 6 input, rest output
	PORTE |= (1<<5)|(1<<2)|(1<<3); // 5th LED off
	DDRA = 0x00; // make PA input
	PORTA = 0xFF; // enable pull up on PA
	
	TCNT0 = -125;
	TCCR0A = 0x00;
	TCCR0B = 0x04;
	TIMSK0 = 0x01;
	
	TCNT1H = (-31250)>>8;
	TCNT1L = (-31250)&0xFF;
	TCCR1A = 0x00;
	TCCR1B = 0x04;
	
	USART_Init(BAUDRATE);
	sei();
	
	iterations = 0;
	halfPer = 0;
	
	unsigned char message1;
	
	while (1) {
		if (bit_is_clear(PINA, PA0)) { // start button
			mode = 1;
			sound(mode);
		} else if (bit_is_clear(PINA, PA1)) { // stop button
			mode = 0;
			sound(mode);
			USART_TxChar(total_time);
			total_time = 0;
		} else if (bit_is_clear(PINA, PA2)) { // clear button
			mode = -1;
		} else if (bit_is_clear(PINE, PE6)) { // timer mode
			mode = -1;
			while (bit_is_clear(PINE, PE6)) {} // wait until release
			timer();
		}
		message1 = USART_RxChar();  //check to see if user wants to know mode
		if(message1 == 'M'){   //user must inquire 'M' for a mode update
			PORTE &= ~(1<<5); // timer LED on
			sound(mode);
			if(mode == 1){
				message = 'S';  //system responds with 'S' if in Stopwatch
				USART_TxChar(message);
			}
			if(mode == 2){
				message = 'T';  //system responds with 'T' if in Timer
				USART_TxChar(message);
			}
		}
	}
}

void alarm() {
	TIMSK1 = 0x01;
	sei();
	mode = 0;
	while (!bit_is_clear(PINA, PA1)){
		PORTE ^= 0x9F;  //toggle output
		TCNT2 = -175;
		TCCR2A = 0x00;
		TCCR2B = 0x04;
		while ((TIFR2 & (1<<TOV2)) == 0);
		TCCR2A = 0x00;
		TCCR2B = 0x00;
		TIFR2 = 0x1;
	}
	mode = 0;
	TIMSK1 = 0x00;
	tenth = 0;
	ones = 0;
	PORTD = 0xFF;
	return;
}

void timer() {
	PORTE &= ~(1<<5); // timer LED on
	mode = 0; // stopped
	tenth = 0;
	ones = 0;
	while (!bit_is_clear(PINE, PE6)) { // read input
		PORTD ^= (ones*16 + tenth);
		if (bit_is_clear(PINA, PA0)) { // decrease tenth init value
			if (tenth == 0) {
				tenth = 9;
			} else {
				tenth--;
			}
			while (bit_is_clear(PINA, PA0)) {}
		} else if (bit_is_clear(PINA, PA1)) { // increase tenth init value
			if (tenth == 9) {
				tenth = 0;
			} else {
				tenth++;
			}
			while (bit_is_clear(PINA, PA1)) {}
		} else if (bit_is_clear(PINA, PA5)) { // decrease ones init value
			if (ones == 0) {
				ones = 9;
			} else {
				ones--;
			}
			while (bit_is_clear(PINA, PA5)) {}
		} else if (bit_is_clear(PINA, PA6)) { // increase ones init value
			if (ones == 9) {
				ones = 0;
			} else {
				ones++;
			}
			while (bit_is_clear(PINA, PA6)) {}
		}
		PORTD ^= (ones*16 + tenth);
	}
	while (bit_is_clear(PINE, PE6)) {} // confirm input
	while(1) {
		if (bit_is_clear(PINA, PA0)) { // start timer
			mode = 2;
		} else if (bit_is_clear(PINA, PA1)) { // stop
			mode = 0;
		} else if (bit_is_clear(PINA, PA2)) { // clear
			mode = -1;
		} else if (bit_is_clear(PINE, PE6)) { // exit timer
			PORTE |= (1<<5); // timer LED off
			mode = -1;
			while (bit_is_clear(PINE, PE6)) {} // wait until release
			return;
		}
	}
}

void sound(int mode) {
	if (mode == 1) { // generates sound for start
		for (int i = 0; i < 4; i++) {
			PORTE ^= 0xBF;  // toggle output
			TCNT2 = -175;
			TCCR2A = 0x00;
			TCCR2B = 0x04;
			while ((TIFR2 & (1<<TOV2)) == 0);
			TCCR2A = 0x00;
			TCCR2B = 0x00;
			TIFR2 = 0x1;
		}
	} else if (mode == 0) { // generates sound for stop
		for (int i = 0; i < 2; i++) {
			PORTE ^= 0xBF;  // toggle output
			TCNT2 = -200;
			TCCR2A = 0x00;
			TCCR2B = 0x04;
			while ((TIFR2 & (1<<TOV2)) == 0);
			TCCR2A = 0x00;
			TCCR2B = 0x00;
			TIFR2 = 0x1;
		}
	}
	return;
}

void USART_Init(unsigned long BR) {
	UCSR2B |= (1 << RXEN) | (1 << TXEN);
	UCSR2C |= (1 << UCSZ1) | (1 << UCSZ0);

	UCSR2C &= ~(1<<USBS);
	
	// enable interrupts for rxc
	UCSR2B |= (1<<RXCIE)|(0<<UDRIE);
	
	unsigned int my_ubrr = (F_CPU/(16*BR)) - 1;
	
	UBRR2L = my_ubrr;
	UBRR2H = (my_ubrr >> 8);
}

unsigned char USART_RxChar() {	
	if ((UCSR2A & (1 << RXC))) {
		return UDR2;
	} else {
		return '\0';
	}
}

void USART_TxChar(char data) {
	UDR2 = data;
	while (!(UCSR2A & (1 << UDRE)));
}

ISR (TIMER0_OVF_vect) { // mode interrupt 1/10 of second
	iterations += 1;
	if (iterations < TOT_ITERATIONS) {
		TCNT0 = -125;
	} else if (halfPer < TOT_HALF_PER) {
		PORTD ^= (ones*16 + tenth);
		if (mode == 1) { // stopwatch
			tenth++;
			total_time++;
			if (tenth == 10) {
				tenth = 0;
				ones++;
				if (ones == 10) {
					ones = 0;
				}
			}
		} else if (mode == 2) { // timer
			if (tenth == 0) {
				tenth = 9;
				if (ones == 0) {
					alarm();
				} else {
					ones--;
				}
			} else {
				tenth--;
			}
		} else if (mode == -1) { // clear
			tenth = 0;
			ones = 0;
			PORTD = 0xFF;
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
ISR (USART2_UDRE_vect) {
	UDR2 = 'x'; 
}
ISR (USART2_RX_vect) {
	message = UDR2;
	if (message == 'a') {
		PORTE &= ~(1<<5); // timer LED on
	}
}

ISR (TIMER1_OVF_vect) {
	TCNT1H = (-31250) >> 8;
	TCNT1L = (-31250) & 0xFF;
	PORTD ^= 0xFF;
}