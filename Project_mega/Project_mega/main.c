/*
 * Project_mega.c
 *
 * Created: 20/04/2022 16.10.32
 * Author : nikla
 */ 

#define CHARCOUNT 16
#define PSLENGTH 4
#define TRUE 1
#define FALSE 0
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

static uint8_t keypad_ScanKey();
uint8_t keypad();

const char Password[5] = "1234";

enum States {
	ACTIVATED,
	SETTIMER,
	ASKPASSWORD,
	DEACTIVATED
	};

volatile enum States state = ACTIVATED;

const uint16_t interruptTime = 65534;

volatile uint16_t interruptCount = 0;

void KEYPAD_Init()
{
	M_RowColDirection= C_RowOutputColInput_U8; // Configure Row lines as O/P and Column lines as I/P
}

void KEYPAD_WaitForKeyRelease()
{
	uint8_t key;
	do
	{
		do
		{
			M_ROW=0x0F;           // Pull the ROW lines to low and Column lines high.
			key=M_COL & 0x0F;     // Read the Columns, to check the key press
		}while(key!=0x0F);

		DELAY_ms(1);

		M_ROW=0x0F;           // Pull the ROW lines to low and Column lines high.
		key=M_COL & 0x0F;     // Read the Columns, to check the key press
	}while(key!=0x0F);   // Wait till the Key is released,
	// If no Key is pressed, Column lines will be High(0x0F)
}

void KEYPAD_WaitForKeyPress()
{
	uint8_t var_keyPress_u8;
	do
	{
		do
		{
			M_ROW=0x0F;		  // Pull the ROW lines to low and Column lines high.
			var_keyPress_u8=M_COL & 0x0F;	  // Read the Columns, to check the key press
		}while(var_keyPress_u8==0x0F); // Wait till the Key is pressed,
		// if a Key is pressed the corresponding Column line go low

		DELAY_ms(1);		  // Wait for some time(debounce Time);

		M_ROW=0x0F;		  // After debounce time, perform the above operation
		var_keyPress_u8=M_COL & 0x0F;	  // to ensure the Key press.

	}while(var_keyPress_u8==0x0F);
}

uint8_t KEYPAD_GetKey()
{
	uint8_t var_keyPress_u8;

	KEYPAD_WaitForKeyRelease();    // Wait for the previous key release
	DELAY_ms(1);

	KEYPAD_WaitForKeyPress();      // Wait for the new key press
	var_keyPress_u8 = keypad_ScanKey();        // Scan for the key pressed.

	switch(var_keyPress_u8)                       // Decode the key
	{
		case 0xe7: var_keyPress_u8='*'; break;
		case 0xeb: var_keyPress_u8='7'; break;
		case 0xed: var_keyPress_u8='4'; break;
		case 0xee: var_keyPress_u8='1'; break;
		case 0xd7: var_keyPress_u8='0'; break;
		case 0xdb: var_keyPress_u8='8'; break;
		case 0xdd: var_keyPress_u8='5'; break;
		case 0xde: var_keyPress_u8='2'; break;
		case 0xb7: var_keyPress_u8='#'; break;
		case 0xbb: var_keyPress_u8='9'; break;
		case 0xbd: var_keyPress_u8='6'; break;
		case 0xbe: var_keyPress_u8='3'; break;
		case 0x77: var_keyPress_u8='D'; break;
		case 0x7b: var_keyPress_u8='C'; break;
		case 0x7d: var_keyPress_u8='B'; break;
		case 0x7e: var_keyPress_u8='A'; break;
		default  : var_keyPress_u8='z'; break;
	}
	return(var_keyPress_u8);                      // Return the key
}

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
			uint8_t pw = keypad();
			if (pw == TRUE) sendCommand()
			break;
			
			case DEACTIVATED:
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
			}
			if (count==2)
			{
				b=' ';
				count=count-1;
			}
			if (count==3)
			{
				c=' ';
				count=count-1;
			}
			if (count==0)
			{
				d=' ';
				count=3;
			}
		}
		else()
		{
			
			if (count==0)
			{
				a=y;
				count=count+1;
			}
			if (count==1)
			{
				b=y;
				count=count+1;
			}
			if (count==2)
			{
				c=y;
				count=count+1;
			}
			if (count==3)
			{
				d=y;
				count=0;
			}
		}
	}
	if (a==e && b==f && c==g && d==h)			//Comparison between correct password and inputs
	{
		return 1;								//Return 1 if correct and 0 if incorrect
	}
	else()
	{
		return 0;
	}
	
}