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

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <avr/interrupt.h>

volatile uint8_t spi_recv;

//LED on pin7 for now
void completeAction(uint8_t command)
{
	switch (command)
	{
		case 20:
			PORTD |= (1 << PD7);
			break;
		case 19:
			PORTD &= ~(1 << PD7);
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
		;
	}
	
	return 0;
}
