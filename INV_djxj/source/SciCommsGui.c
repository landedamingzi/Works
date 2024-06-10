//============================================================================
//============================================================================
//
// FILE:	SciCommsGui.c
//
// TITLE:	GP Comms kernel as an interface to external GUI
// 
// Version: 22 April 2009 - Release 1.2 - Internal Release (BRL)
//============================================================================
//============================================================================
#include "PeripheralHeaderIncludes.h"
#include "IQmathLib.h"
#define	PktSize				6
#define	CmdNumber			16
#define	MAX_CMD_NUM			8


// Function prototypes for Command RECEIVE State machine
// ------------------------------------------------------------
void GetCmdByte(void);
void EchoCmdByte(void);
void GetSizeByte(void);
void EchoSizeByte(void);
void GetDataByte(void);
void EchoDataByte(void);
void PackWord(void);
void PackArray(void);
void CmdInterpreter(void);

// Function prototypes for Command Interpreter and dispatcher
// ------------------------------------------------------------
void LifePulseTsk(void);		// 0
void TextSet(void);				// 1
void ButtonSet(void);			// 2
void SliderSet(void);			// 3
void VariableGet(void);			// 4
void ArrayGet(void);			// 5
void DataGet(void);				// 6
void SpareTsk07(void);			// 7
void SpareTsk08(void);			// 8

void SendData(void);

// Variable declarations
void (*RcvTaskPointer)(void); 			// State pointer for Command Packet Receive
void (*CmdDispatcher[CmdNumber])(void);	// Array of pointers to Function (i.e. tasks)

extern int *varSetTxtList[];
extern int *varSetBtnList[];
extern int *varSetSldrList[];
extern int *varGetList[];
extern int *arrayGetList[];
extern int *dataGetList[];

extern int16 CommsOKflg, SerialCommsTimer;




extern void send_char(char a);
extern void scia_send_float(float a);
extern void scia_msg(char * msg);




extern char Frame_headE[4];
extern char Frame_head1[4];
extern char Frame_head2[4];
extern char Frame_end[4];


extern float *ControlParameter[10];
extern char *Button[2];

extern char RX_Data[32];
extern char RX_OKflg;
Uint16 RX_Byte_count = 0;


Uint16	LowByteFlag, SendTaskPtr;
Uint16	RxChar, RxWord;
Uint16	CmdPacket[PktSize];
Uint16	TaskDoneFlag, NumWords, wordsLeftToGet;

Uint16 dataOut;
int16 *memDataPtr;

int16  RcvTskPtrShdw;	// for debug

int16 	delayer;

int16	MemGetPtr;
Uint32	MemGetAddress;
int16	MemGetAmount;

Uint32 Temp;

int32 qq = 0;

void SCIA_Init()
{    
// Note: Assumes Clocks to SCIA are turned on in InitSysCtrl()
// Note: Assumes GPIO pins for SCIA are configured to Primary function    

	int j = 0;

 	SciaRegs.SCICCR.all =0x0007;
	SciaRegs.SCICTL1.all =0x0003;
	SciaRegs.SCICTL1.bit.RXERRINTENA = 1;  //.
	SciaRegs.SCICTL2.bit.TXINTENA =0;
	SciaRegs.SCICTL2.bit.RXBKINTENA =0;
    SciaRegs.SCIHBAUD    =0x0000;
	SciaRegs.SCILBAUD = 0;         //1562.5Kbaud

    SciaRegs.SCIFFTX.all=0xE040;		// ENable FIFO enhancement
//    SciaRegs.SCIFFTX.all=0x8040;		// DISable FIFO enhancement

   SciaRegs.SCIFFRX.all=0x204f;
 //   SciaRegs.SCIFFRX.all=0x0022;        //+
    SciaRegs.SCIFFRX.bit.RXFFIENA = 1;  //+
    SciaRegs.SCIFFCT.all=0x0;
    SciaRegs.SCIFFTX.bit.SCIFFENA =1;   //+
    SciaRegs.SCICTL1.all =0x0023;       // Relinquish SCI from Reset
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1; //+

    SciaRegs.SCIFFRX.bit.RXFFIL = 4;    //FIFO有几个字节就触发中断
    SciaRegs.SCICTL2.bit.TXINTENA =0;
    SciaRegs.SCIPRI.bit.SOFT=0x0;
    SciaRegs.SCIPRI.bit.FREE=0x1;

	RcvTaskPointer = &GetCmdByte;			// Initialize the CmdPacket Rcv Handler state machine ptr
	RcvTskPtrShdw = 1;						// DEBUG
	SendTaskPtr = 0;						// Init to 1st state
	LowByteFlag = 1;						// Start with LSB during Byte-to-Word packing

	dataOut = 0;
	*memDataPtr = 0;

	RcvTskPtrShdw = 0;	// for debug

	delayer = 0;

	MemGetPtr = 0;
	MemGetAddress = 0x00000000;
	MemGetAmount = 0;

	// clear Command Packet
	for (j=0; j<PktSize; j++)
	{
		CmdPacket[j] = 0x0;
	}

	j=0;
	
// init all dispatch Tasks                   
 	CmdDispatcher[0] = LifePulseTsk;             
 	CmdDispatcher[1] = TextSet;                  
 	CmdDispatcher[2] = ButtonSet;                
 	CmdDispatcher[3] = SliderSet;                
 	CmdDispatcher[4] = VariableGet;              
 	CmdDispatcher[5] = ArrayGet;                 
 	CmdDispatcher[6] = DataGet;                  
 	CmdDispatcher[7] = SpareTsk07;               
 	CmdDispatcher[8] = SpareTsk08;               
                                                                 
}

//===========================================================
// Host Command RECEIVE and DISPATCH State Machine
//===========================================================

//======== SM Entry Point =================
void SerialHostComms()
{		
	(*RcvTaskPointer)();	// Call routine pointed to by state pointer
}
//=========================================

void GetCmdByte(void) // Task 1
{
	if (SciaRegs.SCIRXST.bit.RXRDY == 1)	// check if a char has been received
	{
		RxChar = SciaRegs.SCIRXBUF.all;
		RcvTaskPointer = &EchoCmdByte;		// point to next state
		SerialCommsTimer = 0;
		//RcvTskPtrShdw = 2;				// DEBUG
		EchoCmdByte();
	}                                                                      

	else if (SciaRegs.SCIRXST.bit.BRKDT == 1 || SerialCommsTimer > 2500) //~2 s timeout
	{	
	// If break detected or serialport times out, reset SCI
		//--- Needed by some serialports when code is run with an emulator  
 		SciaRegs.SCICCR.all =0x0007;   		// 1 stop bit,  No loopback             
  	                                 		// No parity,8 char bits,           
  	                                  		// async mode, idle-line protocol   
 		SciaRegs.SCICTL1.all =0x0003;  		// enable TX, RX, internal SCICLK,      
  	                                  		// Disable RX ERR, SLEEP, TXWAKE    
 		SciaRegs.SCICTL2.all =0x0000;                                                                                
                                                                              
 		SciaRegs.SCICTL1.all =0x0023;		// Relinquish SCI from Reset        

		asm(" RPT#8 || NOP");
		//---

		SendTaskPtr = 0;					// Init to 1st state	
		SerialCommsTimer = 0;
								                                      
		CommsOKflg = 0;
		RcvTaskPointer = &GetCmdByte;		// go back and wait for new CMD                       
	}
}

void EchoCmdByte(void) // Task 2
{
    if(SciaRegs.SCICTL2.bit.TXRDY == 1)		// is TXBUF empty ?, i.e. TXRDY = 1
    {
    	SciaRegs.SCITXBUF=RxChar;			// if yes, echo back the received char
    	CmdPacket[0] = RxChar;
		RcvTaskPointer = &GetSizeByte;
		//RcvTskPtrShdw = 3;					// DEBUG
		//RcvTaskPointer = &GetCmdByte;		// Un-comment for simple echo test
		SerialCommsTimer = 0;						// Reset Time-out timer
	}

}

void GetSizeByte(void) // Task 3
{
	if (SciaRegs.SCIRXST.bit.RXRDY == 1)	// check if a char has been received
	{
		RxChar = SciaRegs.SCIRXBUF.all;

		RcvTaskPointer = &EchoSizeByte;		// point to next state
		//RcvTskPtrShdw = 4;					// DEBUG
		EchoSizeByte();
	}

	else if (SerialCommsTimer > 1000)		// 1000*1mS = 1.0 sec timeout
	{
		CommsOKflg = 0;
		RcvTaskPointer = &GetCmdByte;		// Abort, go back wait for new CMD
		SerialCommsTimer = 0;
	}
}

void EchoSizeByte(void) // Task 4
{
    if(SciaRegs.SCICTL2.bit.TXRDY == 1)		// is TXBUF empty ?, i.e. TXRDY = 1
    {
    	SciaRegs.SCITXBUF=RxChar;			// if yes, echo back the received char
    	CmdPacket[1] = RxChar;
		RcvTaskPointer = &GetDataByte;
		//RcvTskPtrShdw = 5;				// DEBUG
		//RcvTaskPointer = &GetCmdByte;		// Un-comment for Test
		SerialCommsTimer = 0;				// Reset Time-out timer
    }
}

void GetDataByte(void) // Task 5
{
	if (SciaRegs.SCIRXST.bit.RXRDY == 1)	// check if a char has been received
	{
		RxChar = SciaRegs.SCIRXBUF.all;
		RcvTaskPointer = &EchoDataByte;		// point to next state
		//RcvTskPtrShdw = 6;				// DEBUG
		EchoDataByte();				
	}

	else if (SerialCommsTimer > 500)		// 1000*1mS = 1 sec timeout
	{
		CommsOKflg = 0;
		RcvTaskPointer = &GetCmdByte;		// Abort, go back wait for new CMD
		SerialCommsTimer = 0;
	}
	
}

void EchoDataByte(void) // Task 6
{
    if(SciaRegs.SCICTL2.bit.TXRDY == 1)		// is TXBUF empty ?, i.e. TXRDY = 1
    {
    	SciaRegs.SCITXBUF=RxChar;			// if yes, echo back the received char
		RcvTaskPointer = &PackWord;
		//RcvTskPtrShdw = 7;				// DEBUG
    }
}

void PackWord(void) // expects LSB first then MSB // Task 7
{
	if(LowByteFlag == 1)
	{ 
		RxWord = RxChar;
		LowByteFlag = 0;
		RcvTaskPointer = &GetDataByte;
		//RcvTskPtrShdw = 5;				// DEBUG
		GetDataByte();
	}
	else
	{
		RxWord = RxWord | (RxChar<<8);
		LowByteFlag = 1;
		CmdPacket[2] = RxWord;				// store data in packet
		RcvTaskPointer = &CmdInterpreter;
		//RcvTskPtrShdw = 8;				// DEBUG
		TaskDoneFlag = 0; 					// indicate new task underway	
	}
}

void CmdInterpreter(void) // Task 8
{
	if (TaskDoneFlag == 0)
	{
	 	(*CmdDispatcher[ CmdPacket[0] ] )();  	// dispatch Task					
	}

	// Incase Task never finishes 
	if (SerialCommsTimer > 2500)			// 2500*1mS = 2.5 sec timeout
	{
		CommsOKflg = 0;
		RcvTaskPointer = &GetCmdByte;		// Abort, go back wait for new CMD
		SerialCommsTimer = 0;
	}
	if (TaskDoneFlag == 1)
	{
		RcvTaskPointer = &GetCmdByte;
	 	//RcvTskPtrShdw = 1;   				// DEBUG
	}
}

//===========================================================
// Slave Tasks commanded by Host
//===========================================================
void LifePulseTsk(void) 	// CmdPacket[0] = 0
{
	if (CmdPacket[2]==0x0000 && CmdPacket[1]==0x00)  //LED2-ON
	{
		#if DSP2802x_DEVICE   
		GpioDataRegs.GPASET.bit.GPIO12=1;		
		#else
		GpioDataRegs.GPASET.bit.GPIO31=1;
		#endif
	}
	if (CmdPacket[2]==0x0001 && CmdPacket[1]==0x00)   //LED2-OFF
	{
		#if DSP2802x_DEVICE
		GpioDataRegs.GPACLEAR.bit.GPIO12=1;
		#else
		GpioDataRegs.GPACLEAR.bit.GPIO31=1;
		#endif		
	}
	if (CmdPacket[2]==0x0002 && CmdPacket[1]==0x00)  //LED2-Toggle
	{
		#if DSP2802x_DEVICE
		GpioDataRegs.GPATOGGLE.bit.GPIO12=1;
		#else
		GpioDataRegs.GPATOGGLE.bit.GPIO31=1;
		#endif
	}

	CommsOKflg = 1;
	SerialCommsTimer = 0;
	TaskDoneFlag = 1;
}
//-----------------------------------------------------------
void TextSet(void) // CmdPacket[0] = 1
{	
	*varSetTxtList[CmdPacket[1]] = CmdPacket[2];

	TaskDoneFlag = 1; // indicate Task execution is complete
}
//-----------------------------------------------------------
void ButtonSet(void) 	// CmdPacket[0] = 2
{
	*varSetBtnList[CmdPacket[1]] = CmdPacket[2];

	TaskDoneFlag = 1; // indicate Task execution is complete
}
//-----------------------------------------------------------
void SliderSet(void) 	// CmdPacket[0] = 3
{
	*varSetSldrList[CmdPacket[1]] = CmdPacket[2];

	TaskDoneFlag = 1; // indicate Task execution is complete
}
//-----------------------------------------------------------
void VariableGet(void)		// CmdPacket[0] = 4
{
	SendData();
}
//-----------------------------------------------------------
//Send a Uint16 array one element at a time
void ArrayGet(void)			// CmdPacket[0] = 5
{
	SendData();	
}
//-----------------------------------------------------------
void DataGet(void)			// CmdPacket[0] = 6
{
	switch(MemGetPtr)
	{
		case 0:
			MemGetAddress = CmdPacket[2];
			MemGetPtr = 1;

			wordsLeftToGet = 1;
			SendTaskPtr = 1;
			TaskDoneFlag = 1;
			break;

		case 1:
			Temp = CmdPacket[2];
			MemGetAddress = MemGetAddress + (Temp<<16);
			memDataPtr = (int16*)MemGetAddress;
			dataOut = *memDataPtr;		
			SendData();		

			if(TaskDoneFlag == 1)
			{
				MemGetPtr = 0;
			}
			break;
		}

	//TaskDoneFlag = 1; 	// indicate Task execution is complete
}
//-----------------------------------------------------------
void SpareTsk07(void)		// CmdPacket[0] = 7
{
	TaskDoneFlag = 1; 		// indicate Task execution is complete
}
//-----------------------------------------------------------
void SpareTsk08(void)		// CmdPacket[0] = 8
{
	TaskDoneFlag = 1; 		// indicate Task execution is complete
}
//-----------------------------------------------------------

void SendData(void)
{
	switch(SendTaskPtr)
	{
	case 0:  //initialization 
		if(CmdPacket[0] == 0x04)
		{
			memDataPtr = (int16 *) varGetList[CmdPacket[1]];
		}
		else
		{
			memDataPtr = (int16 *) arrayGetList[CmdPacket[1]];
		}
		
		dataOut = *memDataPtr;
		wordsLeftToGet = CmdPacket[2];
		//Note that case 0 rolls into case 1 (no break)

	case 1:  //send LSB
		if(wordsLeftToGet > 0)
		{
			if (SciaRegs.SCICTL2.bit.TXRDY == 1)
			{
				SciaRegs.SCITXBUF = dataOut & 0x000000FF;
				SendTaskPtr = 2;
			}
			else
			{
				TaskDoneFlag = TaskDoneFlag;
				break;
			}
		}
		else
		{
			SendTaskPtr = 0;
			TaskDoneFlag = 1;
			break;
		}		

	case 2: //send MSB
		if (SciaRegs.SCICTL2.bit.TXRDY == 1)
		{
			SciaRegs.SCITXBUF = (dataOut>>8 & 0x000000FF);

			memDataPtr = memDataPtr + 1;
			dataOut = *memDataPtr;
			wordsLeftToGet = wordsLeftToGet - 1;
			SendTaskPtr = 1;
		}
		break;
	}
}


int16 GetRX_Cmd(void)
{

    if(RX_Data[0]!=0xF0 || RX_Data[1]!=0x00 || RX_Data[2]!=0x00) return -1; //判断报头

    if(RX_Data[3]==0x02)  //判断为写入
    {
            if(RX_Data[5]==1)   //写入到参数数组
            {
                int32 Rxaa =0;
                int32 Rxaa1,Rxaa2,Rxaa3,Rxaa4;
                Rxaa1=(int32)RX_Data[8];
                Rxaa2=(int32)RX_Data[9];
                Rxaa3=(int32)RX_Data[10];
                Rxaa4=(int32)RX_Data[11];
                Rxaa |= Rxaa1;
                Rxaa |= (*(int32 *)&Rxaa2)<<8;
                Rxaa |= (*(int32 *)&Rxaa3)<<16;
                Rxaa |= (*(int32 *)&Rxaa4)<<24;

                float RXbb =*(float *)&Rxaa;
                scia_msg(Frame_headE);
                scia_send_float(RXbb);
                scia_msg(Frame_end);

                *ControlParameter[RX_Data[7]] = RXbb;

            }

            else if(RX_Data[5]==0)          //写入到按钮数组
            {

                *Button[RX_Data[7]] = RX_Data[11];
            }
            else return -1;



    }
    return 0;
}




interrupt void sciaRxFifoIsr(void)
{
 //   Uint16 i;

    RX_Data[RX_Byte_count]=SciaRegs.SCIRXBUF.all;
    RX_Data[RX_Byte_count+1]=SciaRegs.SCIRXBUF.all;
    RX_Data[RX_Byte_count+2]=SciaRegs.SCIRXBUF.all;
    RX_Data[RX_Byte_count+3]=SciaRegs.SCIRXBUF.all;

    if(RX_Data[RX_Byte_count]==0x0F && RX_Data[RX_Byte_count+1]==0x0F &&
            RX_Data[RX_Byte_count+2]==0x0F && RX_Data[RX_Byte_count+3]==0x0F)
    {
        RX_OKflg = 1;
        RX_Byte_count = 0;
    }else
    {
        RX_Byte_count += 4;
    }
    if(RX_Byte_count >= 32)RX_Byte_count = 0;


    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack
}




void send_char(char a)
{
    while(SciaRegs.SCICTL2.bit.TXRDY != 1)
    {
    }
    SciaRegs.SCITXBUF=a;
}
void scia_send_float(float a)
{

    int16 x = 0;
    int32 rx =*(long *)&a;
    x=rx;
    send_char(x);
    x=rx >>8;
    send_char(x);
    x=rx >>16;
    send_char(x);
    x=rx >>24;
    send_char(x);

}

void scia_msg(char * msg)
{
    int iii;
    for(iii=0;iii<4;iii++)
    {
        send_char(msg[iii]);
    }
}

















