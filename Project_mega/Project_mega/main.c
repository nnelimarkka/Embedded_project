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

volatile enum States state = ACTIVATED;

const uint16_t interruptTime = 65534;

volatile uint16_t interruptCount = 0;

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

/* timer/counter1 fires every ~4 seconds*/
ISR (TIMER1_OVF_vect)
{
	interruptCount++;
	if (interruptCount == 5) //~20 second timeout for password input
	{
		sendCommand(20);
		
		interruptCount = 0;
		
		TIMSK1 &= ~(1 << TOIE1); // disable overflow interrupt
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
	
	lcd_clrscr();
	
	lcd_puts("Alarm system");
	
	while (1)
	{
		switch(state)
		{
			case ACTIVATED:
			/* Check PIR value */
			if ((PINH & (1 << PH4)) == (1 << PH4))
			{
				//motion detected
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts("Motion detected");
				state = SETTIMER;
				} else {
				//no motion
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts("No motion");
				state = ACTIVATED;
			}
			break;
			
			case SETTIMER:
			lcd_clearRow(1);
			lcd_gotoxy(0,1);
			lcd_puts("timer start");
			
			cli();
			
			/* set up the 16-bit timer/counter1 */
			PRR0 &= ~(1 << PRTIM1);
			TCNT1 = 0; // reset timer/counter1 register
			TCCR1A = 0x00;
			TCCR1B = 0b00000101;
			
			TIMSK1 |= (1 << TOIE1); // enable overflow interrupt
			sei();
			state = ASKPASSWORD;
			break;
			
			case ASKPASSWORD:
			break;
			
			case DEACTIVATED:
			break;
		}
	}
	return 0;
}