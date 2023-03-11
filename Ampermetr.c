/*
 * Ampermetr.c
 *
 * Created: 11.09.2016 11:21:40
 *  Author: 123
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define NUMBER_DIGITS 4
#define F_CPU 8000000UL

static uint8_t digits[ NUMBER_DIGITS ];
static uint8_t current_digit = 0x00;
volatile uint16_t value = 0;		// ���������� ��� ����������� ���������
volatile static uint16_t click = 0; // ���������� ��� �������� ��������� 
static uint16_t adc_ch5=0x0000;		// ����������  ��� ��� ������ ��������� �� ���
void convert_number_to_decimal_digitc( uint16_t val );

unsigned char number[]= // ����� �����
{
	0x3F,//0
	0x06,//1
	0x5B,//2
	0x4F,//3
	0x66,//4
	0x6D,//5
	0x7D,//6
	0x07,//7
	0x7F,//8
	0x6F //9
};

// ��������������� ����������� ���� � ��������������
void convert_number_to_decimal_digits( uint16_t val )
{
	uint8_t counter;
	uint16_t buffer = val;
	for (counter = 0; counter < NUMBER_DIGITS; counter++)
	{
		digits[counter] = 0;
	}

	counter = 0;

	while( (counter < NUMBER_DIGITS) && ( buffer !=0) )
	{
		digits[counter] = buffer%10;
		buffer = buffer/10;
		counter++;
	}
}

// ����������� ������ ���� �� �������������� ���������
ISR(TIMER0_OVF_vect)
{
	uint8_t dig = digits[current_digit];
	PORTB = number[ dig ];
	PORTC = ( PORTC&~( 0b00001111 )) | ( 1 << current_digit);
	current_digit++;
	if ( current_digit == NUMBER_DIGITS )
	current_digit=0;
	TIFR |= 1 << TOV0;
}

ISR(ADC_vect)
{
	adc_ch5  = ADCL;
	adc_ch5 |= ADCH << 8;
	value = value + adc_ch5;
	click++;
		
	ADCSRA |= 1 << ADSC;
	ADCSRA |= 1 << ADIF;
}

int main (void)
{
	// ����������� ������ ��� ���������� ����������
	DDRB |=0xFF;
	PORTB |=0x00;
	// ������ ��� ���������� ��������
	DDRC |= (1 << PC3 ) | (1 << PC2 ) | (1 << PC1 ) | (1 << PC0 );
	PORTC &= ~((1 << PC3 ) | (1 << PC2 ) | (1 << PC1 ) | (1 << PC0 ));
	// ������ ��� ����������� ������
	DDRD &= ~((1 << PD0) | (1 << PD1));
	PORTD |= (1 << PD0) | (1 << PD1);
	// ����������� ������
	TCCR0 |=(1 << CS01) | (1 << CS00);
	TCCR0 &= ~( 1 << CS02 );
	TIMSK|= 1 << TOIE0;
	TIFR |= 1 << TOV0;
	// ��������� ���
	ADMUX |= (1 << REFS1) | (1 << REFS0); // ��������� �������� �������� ����������
	ADMUX |= (1 << MUX2) | (1 << MUX0);	  // �������� ���� ��� ADC5
	ADMUX &= ~((1 << MUX3) | (1 << MUX1));
	ADMUX  &= ~(1 << ADLAR);
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ���������� ������� 128
	ADCSRA &= ~(1 << ADFR);
	ADCSRA |= 1 << ADIE;
	ADCSRA |= 1 << ADEN;
	ADCSRA |= 1 << ADSC;
	ADCSRA |= 1 << ADIF;
	sei(); // ��������� ���������� ����������

	while(1)
	{
		if( click==256)
		{
			value = (value/256)*(10/4);
			convert_number_to_decimal_digits(value);
			value = 0;
			click = 0;
		}
	}

	return 0;
}
