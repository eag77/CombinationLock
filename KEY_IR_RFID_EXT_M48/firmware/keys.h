uint8_t Code[9] = {0,0,0,1,1,0,0,1,1};
#define TimeKey 20			/*Веремя дребезга контактов mS*/
#define TimeLock 2000		/*Длительность включенного состояния mS*/
#define TimeKeyReset 1500	/*Максимальная длительность между нажатиями mS*/

#define TimeI 0.001			/*период цикла 1mS*/
#define	cyc TimeI * F_CPU
#define	c (cyc / 0x100)
#define	m ((uint32_t)(cyc) % 0x100)
volatile uint8_t EndTimeCycle = 0;

#define DDRS1 DDRC
#define PINS1 PINC
#define BITS1 PORTC1
#define DDRS2 DDRC
#define PINS2 PINC
#define BITS2 PORTC2
#define DDROUT DDRC
#define PORTOUT PORTC
#define BITOUT PORTC3
#define DDRLED DDRB
#define PORTLED PORTB
#define BITLED PORTB4

uint8_t IN1, IN2;
uint8_t Step;
uint32_t TickLock;
uint32_t TRK;
uint32_t Ticks;

static uint8_t GetInputS1()
{
	uint8_t D = PINS1;
	return ((~D & (1<<BITS1))>>BITS1);
}

static uint8_t GetInputS2()
{
	uint8_t D = PINS2;
	return ((~D & (1<<BITS2))>>BITS2);
}

static uint8_t GetInputsFS1()
{
	static uint16_t T = 0;
	static uint8_t F = 0;
	static uint8_t ins1o;
	uint8_t ins1 = GetInputS1();
	if(ins1 != 0)
	{
		if(F != 0)
		{
			T++;
			if(T >= TimeKey)
			{
				F = 0;
				T = 0;
				return 1;
			}
		}
		if((F == 0)&&(ins1 & ~ins1o))
		{
			F = 1;
			T = 0;
		}
	}
	else
	{
		F = 0;
		T = 0;
	}
	ins1o = ins1;
	return 0;
}

static uint8_t GetInputsFS2()
{
	static uint16_t T = 0;
	static uint8_t F = 0;
	static uint8_t ins1o;
	uint8_t ins1 = GetInputS2();
	if(ins1 != 0)
	{
		if(F != 0)
		{
			T++;
			if(T >= TimeKey)
			{
				F = 0;
				T = 0;
				return 1;
			}
		}
		if((F == 0)&&(ins1 & ~ins1o))
		{
			F = 1;
			T = 0;
		}
	}
	else
	{
		F = 0;
		T = 0;
	}
	ins1o = ins1;
	return 0;
}



int main(void)
{
	PORTOUT &= ~(1<<BITOUT);
	DDROUT |= (1<<BITOUT);
	PORTLED &= ~(1<<BITLED);
	DDRLED |= (1<<BITLED);
	DDRS1 &= ~(1<<BITS1);
	DDRS2 &= ~(1<<BITS2);

	Ticks = 0;
	
    while (1) 
    {
		wdt_reset();

		if(GetInputS1()||GetInputS2())
		{
			PORTLED |= (1<<BITLED);
		}
		else
		{
			PORTLED &= ~(1<<BITLED);
		}

		
		if(EndTimeCycle)	//1ms
		{
			EndTimeCycle = 0;
			
			Ticks++;
			
			IN1 = GetInputsFS1();
			IN2 = GetInputsFS2();
			
			if(TRK > 0)
			{
				TRK--;
			}
			else
			{
				Step = 0;
			}

			if((IN1!=0)||(IN2!=0))
			{
				TRK = TimeKeyReset;
				
				if((IN2&1)==Code[Step])
				{
					Step++;
					if(Step >= 9)
					{
						PORTOUT |= (1<<BITOUT);
						TickLock = TimeLock;
						Step = 0;
					}
				}
				else
				{
					Step = 0;
				}
			}

			if(TickLock > 0)
			{
				TickLock--;
			}
			else
			{
				PORTOUT &= ~(1<<BITOUT);
			}
		}
    }
}
