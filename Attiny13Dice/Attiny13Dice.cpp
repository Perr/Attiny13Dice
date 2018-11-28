/*
 * Attiny13Dice.cpp
 *
 * Created: 15-9-2013 0:51:19
 *  Author: AgentP
 */ 

//#define F_CPU 9600000UL 9.6Mhz / 8 clock

#include <avr/io.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#define DS_PORT    PORTB
#define DS_PIN     3
#define ST_CP_PORT PORTB
#define ST_CP_PIN  4
#define SH_CP_PORT PORTB
#define SH_CP_PIN  0

#define DS_low()  DS_PORT&=~_BV(DS_PIN)
#define DS_high() DS_PORT|=_BV(DS_PIN)
#define ST_CP_low()  ST_CP_PORT&=~_BV(ST_CP_PIN)
#define ST_CP_high() ST_CP_PORT|=_BV(ST_CP_PIN)
#define SH_CP_low()  SH_CP_PORT&=~_BV(SH_CP_PIN)
#define SH_CP_high() SH_CP_PORT|=_BV(SH_CP_PIN)

//Define functions
//===============================================
void WriteNumbers(char n1, char n2);
unsigned char NumberToByte(char number);
void output_led_state(unsigned char __led_state);
unsigned char GetRandomDice();
void Delay(unsigned char);
void GoSleep();
//===============================================

volatile unsigned short timer_shutdown_count = 0;

int main (void)
{
	wdt_enable(WDTO_250MS); //wd on,15ms
	WDTCR = 0b00001101;
	
	DDRB  = 0b00111101; //1 = output, 0 = input
	PORTB = 0b00100110;
	
	TCCR0B = 0b00000011;
	
	TIMSK0 |=1<<TOIE0;
	sei();
	
	if(bit_is_set(PINB, 1)) //if NOT button pressed
	{
		GoSleep();
	}
	
	unsigned char mode = 0;
	
	while(1)
	{
		wdt_reset();
		if(!bit_is_set(PINB, 1)) //if button pressed
		{
			timer_shutdown_count = 0;
			unsigned char delay = 0;
			do 
			{
				wdt_reset();
				if(mode == 0)
				{
					WriteNumbers(GetRandomDice(), GetRandomDice());
				}
				else if (mode == 1)
				{
					WriteNumbers(GetRandomDice(), 0);
				}
				else if (mode == 2)
				{
					WriteNumbers(0, GetRandomDice());
				}
				if(delay < 50)
				{
					if(!bit_is_set(PINB, 1)) //if button pressed
					{
						rand(); //possibly niet nodig
						continue;
					}
					//_delay_ms(20);
					Delay(1); //20ms
				}
				else if(delay < 62)
				{
					//_delay_ms(100);
					Delay(7); //100ms
				}
				else// if(delay < 66)
				{
					//_delay_ms(200);
					Delay(15); //200ms
				}
				++delay;
			}
			while(delay < 66);
			
			if(!bit_is_set(PINB, 1)) //if button pressed
			{
				mode++;
				if(mode >= 3)
				{
					mode = 0;
				}
			}
		}
	}
}

void WriteNumbers(char n1, char n2)
{
	output_led_state(NumberToByte(n1) | (NumberToByte(n2) << 4));
}

volatile unsigned char delaytimer = 0;

void Delay(unsigned char time)
{
	delaytimer = 0;
	while (delaytimer < time)
	{
		//wait
	}
}

ISR(TIM0_OVF_vect)
{
	//rand();
	delaytimer++;
	timer_shutdown_count++;
	if(timer_shutdown_count == 4096) //~60 seconds
	{
		WriteNumbers(0, 0);
		//cli(); //irq's off
		//MCUCR = 0b00110000;
		//while(1); //loop
		GoSleep();
	}
}

void GoSleep()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	sleep_disable();
}

unsigned char GetRandomDice()
{
	return (rand() % 6) + 1;
}

unsigned char NumberToByte(char number)
{
	switch(number)
	{
		case 0:
			return 0b00000000;
		
		case 1:
			return 0b00001000;
			
		case 2:
			return 0b00000100;
			
		case 3:
			return 0b00001100;
			
		case 4:
			return 0b00000110;
			
		case 5:
			return 0b00001110;
			
		case 6:
			return 0b00000111;
	}
	return number;
}

void output_led_state(unsigned char __led_state)
{
	SH_CP_low();
	ST_CP_low();
	for (char i = 0; i < 8; i++)
	{
		if (bit_is_set(__led_state, i))  //bit_is_set doesn’t work on unsigned int so we do this instead
		{
			DS_high();
		}
		else
		{
			DS_low();
		}
		
		SH_CP_high();
		SH_CP_low();
	}
	ST_CP_high();
}