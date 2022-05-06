/*
 * Project_uno.c
 *
 * Created: 20/04/2022 16.19.34
 * Author : nikla
 */ 

/* Uno as slave*/

#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

#define TRUE 1
#define FALSE 0

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <avr/interrupt.h>

volatile uint8_t spi_recv;

volatile uint8_t buzzerEnable = FALSE;

void PWM(uint8_t x);


void completeAction(uint8_t command)
{
	switch (command)
	{
		case 20:
			buzzerEnable = TRUE;
			PWM(1); //enable buzzer
			break;
		case 19:
			buzzerEnable = FALSE;
			PWM(0); //disable buzzer
			break;
		default:
			break;
	}
}

ISR (SPI_STC_vect)
{
	spi_recv = SPDR;
	completeAction(spi_recv);
}

ISR (TIMER1_COMPA_vect)
{
	TCNT1 = 0; // reset timer counter
}

void PWM(uint8_t x)
{
	
	DDRB |= (1 << PB1); // OC1A is located in digital pin 9
	
	// Enable interrupts
	sei();
	
	
	TCCR1B  = 0; 
	TCCR1A  = 0;
	TCNT1   = 0;
	
	TCCR1A |= (1 << 6); 
	
	
	TCCR1A |= (1 << 0); 
	TCCR1B |= (1 << 4); 
	
	TIMSK1 |= (1 << 1); // enable compare match A interrupt
	
	OCR1A = (x==1) ? 2440 : 0;  
	OCR1B = (x==1) ? 500 : 0;
	return;
}

int
main(void)
{
	/* set MISO as output, pin 12 (PB4)*/
	DDRB  = (1 << PB4);
	/* set SPI enable */
	SPCR  = (1 << 6);
	
	SPCR |= (1<<SPIE); // Enable SPI Interrupt
	
	DDRD |= (1 << PD7); //LED output

	sei();
	
	/* send message to master and receive message from master */
	while (1)
	{
		while(buzzerEnable == TRUE)
		{
			/* enable timer/counter1 */
			TCCR1B |= (1 << 0); // set prescaling to 1 (no prescaling)
			
		}
		TIMSK1 &= ~(1 << 1); // disable compare match A interrupt
	}
	
	return 0;
}
