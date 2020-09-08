//RC522	Arduino	M48
//SDA	D10
//SCK	D13
//MOSI	D11
//MISO	D12
//IRQ	-
//GND
//RST	D9
//+3.3
//
//

#ifndef RFID_H_
#define RFID_H_

#include "RC522.h"


uchar RFID_SerNum[MAX_LEN];
uchar status;
uint32_t RFID_CountTime = 0;

static void RFID_INIT()
{
	Write_MFRC522(CommandReg, PCD_RESETPHASE);	//MFRC522_Reset
	Write_MFRC522(TModeReg, 0x8D);				//Tauto=1; f(Timer) = 6.78MHz/TPreScaler
	Write_MFRC522(TPrescalerReg, 0x3E);			//TModeReg[3..0] + TPrescalerReg
	Write_MFRC522(TReloadRegL, 30);
	Write_MFRC522(TReloadRegH, 0);
	Write_MFRC522(TxAutoReg, 0x40);				//100%ASK
	Write_MFRC522(ModeReg, 0x3D);				//CRC Initial value 0x6363        ???
	//AntennaOn
	if (!(Read_MFRC522(TxControlReg) & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}

	_delay_ms(1);
}

/*int compare(uchar* a, uchar* b, uchar length){
	for(int i=0; i<length; i++){
		if(a[i]!=b[i])
			return 1;
	}
	return 0;
}*/

static int RFID_CYCLE()
{
	int res = 0;
	status = MFRC522_Request(PICC_REQIDL, RFID_SerNum);
	status = MFRC522_Anticoll(RFID_SerNum);

	if (status == MI_OK)
	{
		SetFormatRDM630();
		res = 1;
	}
	else
	{
		MFRC522_Halt();
		_delay_ms(50);
	}
	return res;	
}

#endif