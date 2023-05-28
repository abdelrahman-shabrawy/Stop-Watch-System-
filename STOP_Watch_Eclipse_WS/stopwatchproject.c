/*
 * *********************************************************************  *
 * Name       :		Mini project 2
 * Author	  :		Abdelrahman	Shabrawy
 * Description:		Vehicle Control system
 * Date		  :		22/05/2023
 *
 * * *********************************************************************  *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char num_sec1  = 0;
unsigned char num_sec2  = 5;
unsigned char num_min1  = 9;
unsigned char num_min2  = 5;
unsigned char num_hour1 = 9;
unsigned char num_hour2 = 9;


/* External INT0 Interrupt Service Routine */
ISR(INT0_vect)
{
	//Reset the Stop watch
	num_sec1  = 0;
	num_sec2  = 0;
	num_min1  = 0;
	num_min2  = 0;
	num_hour1 = 0;
	num_hour2 = 0;

	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);

}

/* External INT0 enable and configuration function */
void INT0_Init(void)
{
	DDRD  &= (~(1<<PD2));               // Configure INT0/PD2 as input pin
	PORTD |=(1<<PD2);					// ENABLE internal pull-up resistor
	// Trigger INT0 with the FALLING edge
	MCUCR |= (1<<ISC01) ;
	MCUCR &= (~(1<<ISC00)) ;
	GICR  |= (1<<INT0);                 // Enable external interrupt pin INT0

}

/* External INT1 Interrupt Service Routine */
ISR(INT1_vect)
{
	// STOP THE CLOCK
	TCCR1B=0;

}

void INT1_Init(void)
{
	DDRD  &= (~(1<<PD3));  			   // Configure INT1/PD3 as input pin
	MCUCR |= (1<<ISC10) |(1<<ISC11);   // Trigger INT1 with the rising edge
	GICR  |= (1<<INT1);    			   // Enable external interrupt pin INT1

}

/* External INT2 Interrupt Service Routine */
ISR(INT2_vect)
{
	// Resume the clock
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
}

/* External INT2 enable and configuration function */
void INT2_Init(void)
{
	DDRB   &= (~(1<<PB2));   // Configure INT2/PB2 as input pin
	PORTB  |=(1<<PB2);		 // ENABLE internal pull-up resistor
	MCUCSR &= (~(1<<ISC2));  // Trigger INT2 with the FALLING edge
	GICR   |= (1<<INT2);	 // Enable external interrupt pin INT2

}

/* Interrupt Service Routine for timer1 compare mode */
ISR(TIMER1_COMPA_vect)
{
	num_sec1++;

	if(num_sec1 > 9)
	{
		num_sec1 = 0;
		num_sec2++;
	}

	if ((num_sec2 == 6) && (num_sec1 == 0))
	{

		num_sec2 = 0;
		num_min1++;

		if(num_min1 > 9)
		{
			num_min1 = 0;
			num_min2++;

		}

	}

	if((num_min2 == 6) && (num_min1 == 0))
	{
		num_min2 = 0;
		num_hour1++;

		if(num_hour1 > 9)
		{
			num_hour1 = 0;
			num_hour2++;
			if(num_hour2 > 9)
			{
				num_hour2 = 0;
			}
		}

	}

}

/*
 * For System clock=1Mhz and prescale F_CPU/1024.
 * Timer frequency will be around 1khz, Ttimer = 1ms
 * So we just need 1000 counts to get 1s period.
 * Compare interrupt will be generated every 1s.
 */
void Timer1_CTC_Init(void)
{
	TCNT1 = 0;		/* Set timer1 initial count to zero */

	OCR1A = 1000 ;    /* Set the Compare value to 1 second (1000) */

	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */

	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A = (1<<FOC1A);
	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/1024 CS10=1 CS11=0 CS12=1
	 */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
}

int main(void)

{
	/********** Initialization Code **********/
	SREG   |= (1<<7);     	/* Enable global interrupts in MC */
	INT0_Init();
	INT1_Init();
	INT2_Init();
	Timer1_CTC_Init();

	// 7TH SEGMENT
	DDRC |= 0x0F; // configure first four pins of PORTC as output pins
	// initialize the 7-segment with value 0 by clear the first four bits in PORTC
	PORTC &= 0xF0;

	DDRA  |= 0X1F;        // configure first 5 pins  in PORTA as output pin

	// Set first 5 pins in PORTA with value 1 to enable the transistor to connect the first 7-segment common to the VCC (common ANODE)
	PORTA &= 0XC0;
	//PORTA |= 0X3F;

	while(1)
	{
		/********** Application Code **********/
		// Display the required number on the 7-segment

		PORTA &= 0Xc1;
		PORTA|=(1<<PA0);
		PORTC = (PORTC & 0xF0) | (num_sec1 & 0x0F);
		_delay_ms(2);

		PORTA &= 0XC2;
		PORTA|=(1<<PA1);
		PORTC = (PORTC & 0xF0) | (num_sec2 & 0x0F);
		_delay_ms(2);

		PORTA &= 0XC4;
		PORTA|=(1<<PA2);
		PORTC = (PORTC & 0xF0) | (num_min1 & 0x0F);
		_delay_ms(2);

		PORTA &= 0XC8;
		PORTA|=(1<<PA3);
		PORTC = (PORTC & 0xF0) | (num_min2 & 0x0F);
		_delay_ms(2);

		PORTA &= 0XD0;
		PORTA|=(1<<PA4);
		PORTC = (PORTC & 0xF0) | (num_hour1 & 0x0F);
		_delay_ms(2);

		PORTA &= 0XE0;
		PORTA|=(1<<PA5);
		PORTC = (PORTC & 0xF0) | (num_hour2 & 0x0F);
		_delay_ms(2);

	}
}
