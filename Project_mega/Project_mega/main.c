/*
 * Project_mega.c
 *
 * Created: 20/04/2022 16.10.32
 * Author : nikla
 */ 

#define CHARCOUNT 16

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
#include "lcd.h"

enum States {
	ACTIVATED,
	SETTIMER,
	ASKPASSWORD,
	DEACTIVATED
	};

volatile States state = ACTIVATED;

void sendCommand(uint8_t command)
{
	/* send byte to slave */
	
	PORTB &= ~(1 << PB0); // SS LOW
	
	
	SPDR = command; // send byte using SPI data register
	
	while(!(SPSR & (1 << SPIF)))
	{
		/* wait until the transmission is complete */
		;
	}
	
	
	PORTB |= (1 << PB0); // SS HIGH
}

void lcd_clearRow(uint8_t row)
{
	lcd_gotoxy(0,row);
	for (int i = 0; i< CHARCOUNT; i++)
	{
		lcd_puts(" ");
	}
}

int
main(void)
{
	/* set SS, MOSI and SCK as output, pins 53 (PB0), 51 (PB2) and 52 (PB1) */
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2); // SS as output
	/* set SPI enable and master/slave select, making MEGA the master */
	SPCR |= (1 << 6) | (1 << 4);
	/* set SPI clock rate to 1 MHz */
	SPCR |= (1 << 0);
	
	/* Set PIR as input */
	DDRH &= ~(1 << PH4);
	
	/* Initialize lcd */
	lcd_init(LCD_DISP_ON);
	
	lcd_puts("Alarm system");

	/* send message to slave */
	while (1)
	{
		
		/* Check PIR value */
		if ((PINH & (1 << PH4)) == (1 << PH4))
		{
			//motion detected
			sendCommand(20);
			lcd_clearRow(1);
			lcd_gotoxy(0,1);
			lcd_puts("Motion detected");
			_delay_ms(100);
		} else {
			//no motion
			sendCommand(19);
			lcd_clearRow(1);
			lcd_gotoxy(0,1);
			lcd_puts("No motion");
			_delay_ms(100);
		}
		
	}
	
	return 0;
}