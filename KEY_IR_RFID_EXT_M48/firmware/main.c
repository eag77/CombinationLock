#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "spi.h"
#include "RFID.h"

#define DDRLED1 DDRC
#define PORTLED1 PORTC
#define BITLED1 PORTC3
#define LED1_ON	PORTLED1 |= (1<<BITLED1)
#define LED1_OFF PORTLED1 &= ~(1<<BITLED1)

#define DDRLED2 DDRC
#define PORTLED2 PORTC
#define BITLED2 PORTC4
#define LED2_ON	PORTLED2 |= (1<<BITLED2)
#define LED2_OFF PORTLED2 &= ~(1<<BITLED2)

#define RF_RES PORTC1
#define RF_NS PORTB0

#define IN_IR (PINC & (1 << PORTC2))

uint8_t IR_State;
uint8_t IR_SerNum[4];
volatile uint32_t OldTicks;

#define TimeI 0.001			/*период цикла 10mS*/
#define	cyc TimeI * F_CPU
#define	c (cyc / 0x100)
#define	m ((uint32_t)(cyc) % 0x100)

volatile uint8_t EndTimeCycle = 0;
volatile uint32_t Ticks;
volatile uint32_t NewTime;
volatile uint16_t IR_Delay;

int main(void)
{
	TCNT0 = 0;
	TIFR0 |= (1<<OCF0A)|(1<<TOV0);	//Clear flags
	TIMSK0 |= (1<<TOIE0);
	TCCR0B = 1;

	TCNT2 = 0;
	TCCR2A = 0;
//	TCCR2C = 0;
	TCCR2B = 0;//7;	//16MHz/1024=15625Hz; 256=16.384ms 1=64us
	TIMSK2 = 0;//(1<<TOIE2);

	DDRB = (1<<RF_NS);
	PORTB = (1<<RF_NS);
	DDRC = (1<<BITLED2)|(1<<BITLED1)|(1<<RF_RES);
	PORTC = (0<<BITLED2)|(0<<BITLED1)|(1<<RF_RES);
	LED2_ON;

	_delay_ms(500);
	
	wdt_enable(WDTO_1S);
	wdt_reset();
	
	init_uart(57600);
//    stdout = &mystdout;
	_delay_ms(100);
//	rs485_printf("CL EXT\n");
	SPI_Init();
	sei();
	Ticks = 0;
	NewTime = Ticks+100;
	RFID_INIT();

	IR_State = 0;
	EndTimeCycle = 0;
	USART_State = 0;
	PCIFR = 0xFF;
	PCICR = (1<<PCIE1);
	PCMSK1 = (1<<PCINT10);

//	rs485_printf("EXT1\n");
	LED2_OFF;
	IR_Delay = 1000;

    while (1) 
    {
		sei();
		wdt_reset();
		
		if(EndTimeCycle)	//1ms
		{
			EndTimeCycle = 0;
			
/*?			if (IR_Delay == 1)
			{
				PCIFR = 0xFF;
				PCICR = (1<<PCIE1);
			}
			
			if(IR_Delay > 0)
			{
				IR_Delay--;
			}*/
			
			if(IR_State==4)
			{
				TXE_H;
				printf("IR");
				uint8_t crc=0;
				for(int i=0; i<3; i++)
				{
					uart_transmite(IR_SerNum[i]);
					crc+=IR_SerNum[i];
				}
				uart_transmite(crc);
				printf("\n");
				TXE_L;
				IR_State=0;
				IR_Delay=1000;
			}

			if(Ticks >= NewTime)	//100ms
			{
				NewTime = Ticks+100;

				RFID_INIT();
				if(RFID_CYCLE())
				{
					TXE_H;
					printf("RF");
					uint8_t crc=0;
					for(int i=0; i<5; i++)
					{
						uart_transmite(RFID_SerNum[i]);
						crc+=RFID_SerNum[i];
					}
					uart_transmite(crc);
					printf("\n");
					TXE_L;
				}
			}
			
			if(USART_State)
			{
				USART_State=0;
				(RXLed&1)!=0?(LED1_ON):(LED1_OFF);
				(RXLed&2)!=0?(LED2_ON):(LED2_OFF);
			}
		}
    }
}


volatile static uint32_t count = 0;

ISR(TIMER0_OVF_vect)
{
	count++;
	if(count == (uint32_t)(c))
	{
		EndTimeCycle = 1;	//флаг для основного цикла программы
		OCR0A = m-20;
		TIFR0 |= (1<<OCF0A);
		TIMSK0 |= (1<<OCIE0A);
		count = 0;
		return;
	}
	TIMSK0 |= (1<<TOIE0);
}

ISR(TIMER0_COMPA_vect)
{
	TCNT0 = 0;		//сбрасываем таймер для отсчета 1мс
	count = 0;
	EndTimeCycle = 1;	//флаг для основного цикла программы
	TIMSK0 &= ~(1<<OCIE0A);
	TIMSK0 |= (1<<TOIE0);
	Ticks++;
}

ISR(PCINT1_vect)
{
	switch(IR_State)
	{
		case 0:
			if(IN_IR == 0)
			{
				TCNT2 = 0;
				TCCR2A = 0;
	//			TCCR2C = 0;
				TCCR2B = 7;	//16MHz/1024=15625Hz; 256=16.384ms 1=64us
				TIMSK2 = (1<<TOIE2);
				IR_State = 1;
			}
			break;
		case 1:
			if(IN_IR != 0)
			{
				IR_SerNum[0]=TCNT2;//TIM16_ReadTCNT1();
				IR_State = 2;
			}
			break;
		case 2:
			if(IN_IR == 0)
			{
				IR_SerNum[1]=TCNT2;//TIM16_ReadTCNT1();
				IR_State = 3;
			}
			break;
		case 3:
			if(IN_IR != 0)
			{
				IR_SerNum[2]=TCNT2;//TIM16_ReadTCNT1();
				TCCR2B = 0;	//Отключаем счёт
				IR_State = 4;
//				PCICR = 0;	//Отключаем прерывание
//				LED1_OFF;
			}
			break;
		default:
		break;
	}
	
}


ISR(TIMER2_OVF_vect)
{//16,384mS
	TCCR2B = 0;
	IR_State = 0;
//	LED1_ON;
}