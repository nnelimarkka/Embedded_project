/*
 * Project_mega.c
 *
 * Created: 20/04/2022 16.10.32
 * Author : Niklas, Rasmus & Riku
 */ 

#define CHARCOUNT 16
#define PSLENGTH 4
#define ALARM 20
#define DEACTIVATE 19

/* Mega is master */
/* Keypad code/library and lcd code/library was taken from provided additional materials */

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
#include "keypad.h"
#include "delay.h"

uint8_t keypad();

const char Password[5] = "1234";

enum States {
	ACTIVATED,
	SETTIMER,
	ASKPASSWORD,
	DEACTIVATED
	};

volatile enum States state = ACTIVATED;

volatile uint16_t interruptCount = 0;

volatile uint8_t isIncorrectPassword = 0;

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
	if (interruptCount == 5) //~26 second timeout for password input
	{
		sendCommand(ALARM);
		
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
	
	KEYPAD_Init();
	
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
			lcd_puts("Timer start");
			
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
			if (isIncorrectPassword == 0)
			{
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts("Give password");
			}
			
			uint8_t pw = keypad();
			if (pw == 1)
			{
				TIMSK1 &= ~(1 << TOIE1); // disable overflow interrupt to prevent timer interrupt
				interruptCount = 0;
				isIncorrectPassword = 0;
				sendCommand(DEACTIVATE);
				state = DEACTIVATED;
			} else 
			{
				isIncorrectPassword = 1;
				sendCommand(ALARM);
				state = ASKPASSWORD;
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts("Wrong password!");
			}
			break;
			
			case DEACTIVATED:
			lcd_clearRow(1);
			lcd_gotoxy(0,1);
			lcd_puts("Alarm deactivated");
			
			uint8_t x =KEYPAD_GetKey();
			if (x == 'A') state = ACTIVATED; //Rearm by pressing 'A' on keypad
			else state = DEACTIVATED;
			break;
		}
	}
	return 0;
}

uint8_t keypad() 
{
	uint8_t a =' ';								//Define variables for storing key presses (a-d)
	uint8_t b =' ';
	uint8_t c =' ';
	uint8_t d =' ';
	uint8_t count =0;							//Define counter to switch between variables
	uint8_t e =' ';								
	uint8_t f =' ';
	uint8_t g =' ';
	uint8_t h =' ';
	
	char currentPassword[5] = {' ', ' ', ' ', ' ', '\0'};
	
	for (int i=0;i<PSLENGTH;i++) //Define the correct password (e-h)
	{
		if (i==0) e = Password[i];
		if (i==1) f = Password[i];
		if (i==2) g = Password[i];
		if (i==3) h = Password[i];
	}
	while(1)
	{
		uint8_t y =KEYPAD_GetKey();				//Receive inputs from keypad
		if (y=='#')								//# is our submit button
		{
			break;
		}
		if (y=='*')								//* is our backspace
		{
			if (count==1)
			{
				a=' ';
				count=count-1;
				currentPassword[count] = a;
				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==2)
			{
				b=' ';
				count=count-1;
				currentPassword[count] = b;
				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==3)
			{
				c=' ';
				count=count-1;
				currentPassword[count] = c;

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==4)
			{
				d=' ';
				count=count-1;
				currentPassword[count] = d;

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
		}
		else
		{
			
			if (count==0)
			{
				a=y;
				currentPassword[count] = a;
				count=count+1;
				

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==1)
			{
				b=y;
				currentPassword[count] = b;
				count=count+1;
				

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==2)
			{
				c=y;
				currentPassword[count] = c;
				count=count+1;
				

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count==3)
			{
				d=y;
				currentPassword[count] = d;
				count=count+1;
				

				
				lcd_clearRow(1);
				lcd_gotoxy(0,1);
				lcd_puts(currentPassword);
			}
			else if (count == 4)
			{
				return 0; // password is too long and it cannot be correct
			}
		}
	}
	if (a==e && b==f && c==g && d==h)			//Comparison between correct password and inputs
	{
		return 1;								//Return 1 if correct and 0 if incorrect
	}
	else
	{
		return 0;
	}
	
}
