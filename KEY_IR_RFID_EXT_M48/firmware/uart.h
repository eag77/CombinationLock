#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define TXEPIN PORTD2
#define TXEPORT PORTD
#define TXEDDR DDRD
#define TXE_H TXEPORT |= (1<<TXEPIN);
#define TXE_L TXEPORT &= ~(1<<TXEPIN);
#define RX_ON UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)
#define RX_OFF UCSR0B = (0<<RXEN0)|(1<<TXEN0)|(0<<RXCIE0)

char StrLed[6] = "LEDx\r\n";
char RXbuf[10];
uint8_t RXLed;
uint8_t RXCount;
uint8_t USART_State = 0;

static uint8_t rs485_len_indata;


int uart_putchar(char c, FILE *stream)
{
	//	TXEPORT |= (1<<TXEPIN);
	//	UCSR0B = (0<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	if (c == '\n')
	uart_putchar('\r', stream);
	while(!(UCSR0A & (1<<UDRE0)));
	UCSR0A |= (1<<TXC0);
	UDR0 = c;
	wdt_reset();
	while(!(UCSR0A & (1<<TXC0)));
	//	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	//	TXEPORT &= ~(1<<TXEPIN);
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

static void init_uart(uint32_t BAUD)
{
	rs485_len_indata = 0;
	TXEDDR |= (1<<TXEPIN);
	TXE_L;
//	uint16_t ubrr = (uint32_t)F_CPU/16/BAUD-1;
volatile uint32_t ubrr = (F_CPU/(BAUD*16))-1;
	//Set baud rate
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	// Set frame format: 8data, 2stop bit
	UCSR0C = (0<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
	UCSR0A = 0;
    stdout = &mystdout;
	RXCount = 0;
}

void rs485_printf(const char* __fmt, ...)
{
	RX_OFF;
	TXE_H;
	printf(__fmt);
	TXE_L;
	RX_ON;
}

void rs485_transmite(char c)
{
	RX_OFF;
	TXE_H;
	while(!(UCSR0A & (1<<UDRE0)));
	UCSR0A |= (1<<TXC0);
	UDR0 = c;
	while(!(UCSR0A & (1<<TXC0)));
	TXE_L;
	RX_ON;
}

int uart_transmite(char c)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UCSR0A |= (1<<TXC0);
	UDR0 = c;
	while(!(UCSR0A & (1<<TXC0)));
	return 0;
}


#define FRAMING_ERROR (1<<FE0)
#define PARITY_ERROR (1<<UPE0)
#define DATA_OVERRUN (1<<DOR0)

ISR(USART_RX_vect)
{
	char status=UCSR0A;
	char data=UDR0;
	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
	{
		RXbuf[RXCount]=data;
		
		if(RXCount==3)
		{
			RXLed = data;
			RXCount++;
		}
		else
			if(StrLed[RXCount]==data)
			{
				RXCount++;
				if(RXCount>=6)
				{
					USART_State = 1;
					RXCount=0;
				}
			}
			else
			{
				RXCount=0;
			}
	}
}

#endif /* UART_H_ */