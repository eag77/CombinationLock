#include <avr/io.h>
#include "spi.h"

void SPI_Init(void)
{
	DDRB |=(1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5); //��������� ������ MOSI,SS,SCK �� �����
	PORTB|=(1<<PORTB2); //���������� "1" �� ����� SS

	SPCR = 0; //�������� ������� SPCR
	SPSR = 0; //�������� ������� SPSR

	SPCR |= (1<<MSTR)|(1<<SPR0); //����� ������, F=Fosc/16
//	SPCR |= (1<<MSTR)|(0<<SPR1)|(0<<SPR0); //����� ������, F=Fosc/4
	SPSR |= (0<<SPI2X); //F=Fosc/8

	SPCR |= (1<<SPE);//�������� SPI
}

char SPI_WR_byte(char data)
{
	PORTB &=~(1<<PORTB2); //���������� "0" �� ����� SS
	SPDR = data; //��������� ����
	while(!(SPSR&(1<<SPIF))) ; //��������� ��������� ��������
	PORTB |=(1<<PORTB2); //���������� "1" �� ����� SS
	return SPDR;
}
