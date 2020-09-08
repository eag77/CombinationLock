#include <avr/io.h>
#include "spi.h"

void SPI_Init(void)
{
	DDRB |=(1<<PORTB2)|(1<<PORTB3)|(1<<PORTB5); //Настроить выводы MOSI,SS,SCK на выход
	PORTB|=(1<<PORTB2); //Установить "1" на линии SS

	SPCR = 0; //Обнулить регистр SPCR
	SPSR = 0; //Обнулить регистр SPSR

	SPCR |= (1<<MSTR)|(1<<SPR0); //Режим мастер, F=Fosc/16
//	SPCR |= (1<<MSTR)|(0<<SPR1)|(0<<SPR0); //Режим мастер, F=Fosc/4
	SPSR |= (0<<SPI2X); //F=Fosc/8

	SPCR |= (1<<SPE);//Включить SPI
}

char SPI_WR_byte(char data)
{
	PORTB &=~(1<<PORTB2); //Установить "0" на линии SS
	SPDR = data; //Отправить байт
	while(!(SPSR&(1<<SPIF))) ; //Дождаться окончания передачи
	PORTB |=(1<<PORTB2); //Установить "1" на линии SS
	return SPDR;
}
