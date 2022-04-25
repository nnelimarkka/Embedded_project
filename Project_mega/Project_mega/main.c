/*
 * Project_mega.c
 *
 * Created: 20/04/2022 16.10.32
 * Author : nikla
 */ 

/* Mega is master */

#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>

int
main(void)
{
	/* set SS, MOSI and SCK as output, pins 53 (PB0), 51 (PB2) and 52 (PB1) */
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2); // SS as output
	/* set SPI enable and master/slave select, making MEGA the master */
	SPCR |= (1 << 6) | (1 << 4);
	/* set SPI clock rate to 1 MHz */
	SPCR |= (1 << 0);

	/* send message to slave */
	while (1)
	{
		/* send byte to slave and receive a byte from slave */
		PORTB &= ~(1 << PB0); // SS LOW
		
		
		SPDR = (uint8_t)20; // send byte using SPI data register
			
		while(!(SPSR & (1 << SPIF)))
		{
			/* wait until the transmission is complete */
			;
		}
			
		//spi_receive_data[spi_data_index] = SPDR; // receive byte from the SPI data register
		
		
		PORTB |= (1 << PB0); // SS HIGH
		
		_delay_ms(5000);
		
		PORTB &= ~(1 << PB0); // SS LOW
		
		
		SPDR = (uint8_t)19; // send byte using SPI data register
		
		while(!(SPSR & (1 << SPIF)))
		{
			/* wait until the transmission is complete */
			;
		}
		
		//spi_receive_data[spi_data_index] = SPDR; // receive byte from the SPI data register
		
		
		PORTB |= (1 << PB0); // SS HIGH
		
		_delay_ms(5000);
	}
	
	return 0;
}