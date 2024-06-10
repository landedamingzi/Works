//----------------------------------------------------------------------------------
//	FILE:			FlashingLeds-Main.C
//
//	Description:	Empty Framework ready for customization
//
//	Version: 		2.0
//
//  Target:  		TMS320F2806x
//
//	Type: 			Device Independent
//
//----------------------------------------------------------------------------------
//  Copyright Texas Instruments � 2004-2010
//----------------------------------------------------------------------------------
//  Revision History:
//----------------------------------------------------------------------------------
//  Date	  | Description / Status
//----------------------------------------------------------------------------------
// 13 July 2010 - BRL
//----------------------------------------------------------------------------------
//
// PLEASE READ - Useful notes about this Project

// Although this project is made up of several files, the most important ones are:
//	 "FlashingLeds-Main.C"	- this file
//		- Application Initialization, Peripheral config,
//		- Application management
//		- Slower background code loops and Task scheduling
//	 "FlashingLeds-DevInit_F28xxx.C
//		- Device Initialization, e.g. Clock, PLL, WD, GPIO mapping
//		- Peripheral clock enables
//		- DevInit file will differ per each F28xxx device series, e.g. F280x, F2833x,
//	 "FlashingLeds-ISR.asm
//		- Assembly level library Macros and any cycle critical functions are found here
//	 "FlashingLeds-Settings.h"
//		- Global defines (settings) project selections are found here
//		- This file is referenced by both C and ASM files.

// Code is made up of sections, e.g. "FUNCTION PROTOTYPES", "VARIABLE DECLARATIONS" ,..etc
//	each section has FRAMEWORK and USER areas.
//  FRAMEWORK areas provide useful ready made "infrastructure" code which for the most part
//	does not need modification, e.g. Task scheduling, ISR call, GUI interface support,...etc
//  USER areas have functional example code which can be modified by USER to fit their appl.
//

//----------------------------------------------------------------------------------
//This program blinks LED3 on the controlCARD at a frequency given by the variable 
// Gui_LedPrd_ms. If an external GUI is connected LD2 will blink as well.
// 
//----------------------------------------------------------------------------------
#include "DPlib.h"

#include "FlashingLeds-Settings.h"
#include "PeripheralHeaderIncludes.h"
#include "F2806x_EPWM_defines.h"

#include "IQmathLib.h"
#include "Solar_IQ.h"
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// FUNCTION PROTOTYPES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// -------------------------------- FRAMEWORK --------------------------------------

void DeviceInit(void);
void SCIA_Init();
void SerialHostComms();
void InitFlash();
void MemCopy();

void PWM_1ch_CNF(int16 n, Uint16 period, int16 mode, int16 phase);
void PWM_ComplPairDB_CNF(int16 n, Uint16 period, int16 mode, int16 phase);
void PWM_ComplPairDB_UpdateDB(int16 n, int16 dbRED, int16 dbFED);
void ADC_SOC_CNF(int ChSel[], int Trigsel[], int ACQPS[], int IntChSel, int mode);
interrupt void sciaRxFifoIsr(void);
void send_char(char a);
void scia_send_float(float a);
void scia_msg(char * msg);
int16 GetRX_Cmd(void);
__interrupt void ADC_INT();

// State Machine function prototypes
//------------------------------------
// Alpha states
void A0(void);	//state A0
void B0(void);	//state B0
void C0(void);	//state C0

// A branch states
void A1(void);	//state A1
void A2(void);	//state A2

// B branch states
void B1(void);	//state B1
void B2(void);	//state B2

// C branch states
void C1(void);	//state C1
void C2(void);	//state C2

// Variable declarations
void (*Alpha_State_Ptr)(void);	// Base States pointer
void (*A_Task_Ptr)(void);		// State pointer A branch
void (*B_Task_Ptr)(void);		// State pointer B branch
void (*C_Task_Ptr)(void);		// State pointer C branch


// ---------------------------------- USER -----------------------------------------

//extern void PWM_1ch_CNF(int16 n, Uint16 period, int16 mode, int16 phase);


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// VARIABLE DECLARATIONS - GENERAL
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// -------------------------------- FRAMEWORK --------------------------------------

int16	SerialCommsTimer;
int16	CommsOKflg;


// Used for running BackGround in flash, and ISR in RAM
extern Uint16 *RamfuncsLoadStart, *RamfuncsLoadEnd, *RamfuncsRunStart;


// ---------------------------------- USER -----------------------------------------
int16	Gui_LedPrd_ms;			// LED Prd in ms
int16	LedBlinkTimer;


#define SPLL_SOGI_ 1




/***********************************************************
 * 用户：下面是定义声明
************************************************************/
#define EPWM_FS 10000
#define SYS_CLOCK 100000000



#define PI 3.1415926

//**********************************************************//关于ADC
int     ChSel[16] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    //SOC通道关联
int     TrigSel[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    //触发关联
int     ACQPS[16] =   {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};    //保持时间
float ADC_Data[20];                                         //采集的数据



//**********************************************************//ePWM
volatile struct EPWM_REGS *ePWM[] =
                  { &EPwm1Regs,                              //intentional: (ePWM[0] not used)
                    &EPwm1Regs,
                    &EPwm2Regs,
                    &EPwm3Regs,
                    &EPwm4Regs,
                    &EPwm5Regs,
                    &EPwm6Regs,
                    &EPwm7Regs,
                  };


/***********************************************************
 * **************DPLib相关定义
************************************************************/

#if(SPLL_SOGI_==1)
SPLL_1ph_SOGI_IQ spll1;                                 //创建锁相环模块
#endif
#if(SPLL_SOGI_==0)
SPLL_1ph_IQ spll1;
SPLL_1ph_IQ_NOTCH_COEFF spll_notch_coef1;
#endif



                                                        //用于关联的变量
extern volatile long *PWMDRV_1ch_Duty1;
extern volatile long *PWMDRV_1ch_Duty2;
extern volatile long *PWMDRV_ComplPairDB_Duty6;
extern volatile long *PWMDRV_ComplPairDB_Duty7;
extern volatile long *ADCDRV_1ch_Rlt1;

volatile long Duty,Duty_Boost1,Duty_Boost2,ADC1Out;

float Dutyf = 0,Duty_Boostf1 = 0,Duty_Boostf2 = 0;


float *ControlParameter[10] = {&Dutyf,&Duty_Boostf1,&Duty_Boostf2,0,0,0,0,0,0};
/*
    0.Duty
    1.Duty_Boost1
    2Duty_Boost2

 */

                                                        //各开关量地址，用于串口指令
char Run_Button = 0;
char WaveSend_Button = 1;
char *Button[2] = {&Run_Button,&WaveSend_Button};
/*
 0.总开关
 1.波形发送开关

*/



                                                        //串口调试报头报尾
char Frame_headE[4] = {0xF0, 0x00, 0x00, 0x0E};
char Frame_head1[4] = {0xF0, 0x00, 0x00, 0x01};
char Frame_head2[4] = {0xF0, 0x00, 0x00, 0x02};
char Frame_end[4] = {0x0F, 0x0F, 0x0F, 0x0F};
                                                        //串口调试接收
char RX_Data[32] = {0};             //接收数据
char RX_OKflg = 0;                  //检测到报尾 flg
char ADC_Send_flag = 0;             //正在发送ADC值flg




//PWMDRV_1ch_Duty1 = &net1;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// VARIABLE DECLARATIONS - CCS WatchWindow / GUI support
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// -------------------------------- FRAMEWORK --------------------------------------

//GUI support variables
// sets a limit on the amount of external GUI controls - increase as necessary
int16 	*varSetTxtList[8];				//8 textbox controlled variables
int16 	*varSetBtnList[8];				//8 button controlled variables
int16 	*varSetSldrList[8];				//8 slider controlled variables
int16 	*varGetList[8];					//8 variables sendable to GUI
int16 	*arrayGetList[8];				//8 arrays sendable to GUI				


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// MAIN CODE - starts here
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void main(void)

 {
//=================================================================================
//	INITIALIZATION - General
//=================================================================================

//-------------------------------- FRAMEWORK --------------------------------------


    DeviceInit();	// Device Life support & GPIO
	SCIA_Init();  	// Initalize the Serial Comms A peripheral


// Only used if running from FLASH
// Note that the variable FLASH is defined by the compiler with -d FLASH
// (see the project's Build Properties)
#ifdef FLASH		
// Copy time critical code and Flash setup code to RAM
// The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
// symbols are created by the linker. Refer to the linker files. 

	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
	InitFlash();	// Call the flash wrapper init function
#endif //(FLASH)

// Initialize period for each CPU Timer (used by background loops)
// Timer period definitions found in PeripheralHeaderIncludes.h

	CpuTimer0Regs.PRD.all =  mSec1;		// A tasks
	CpuTimer1Regs.PRD.all =  mSec50;	// B tasks
	CpuTimer2Regs.PRD.all =  mSec500;	// C tasks

// Tasks State-machine init
	Alpha_State_Ptr = &A0;
	A_Task_Ptr = &A1;
	B_Task_Ptr = &B1;
	C_Task_Ptr = &C1;

	CommsOKflg = 0;
	SerialCommsTimer = 0;


// ---------------------------------- USER -----------------------------------------
//  put common initialization/variable definitions here

//	ADCDRV_1ch_Rlt0 =&Net2;

//	CNTL_2P2Z_Fdbk1 =&net2;



//=================================================================================
//	INITIALIZATION - GUI connections
//=================================================================================
// Use this section only if you plan to "Instrument" your application using the 
// Microsoft C# freeware GUI Template provided by TI

	//"Set" variables
	//---------------------------------------
	// assign GUI variable Textboxes to desired "setable" parameter addresses
	//varSetTxtList[0] = &Var1;

	// assign GUI Buttons to desired flag addresses
	//varSetBtnList[0] = &Var2;

	// assign GUI Sliders to desired "setable" parameter addresses
	varSetSldrList[0] = &Gui_LedPrd_ms;


	//"Get" variables
	//---------------------------------------
	// assign a GUI "getable" parameter address
	//varGetList[0] = &Var3;

	// assign a GUI "getable" parameter array address
	// 		only need to set initial position of array, program will run through it 
	//       based on the array length specified in the GUI
	//arrayGetList[0] = &Var4[0];



//==================================================================================
//	INITIALIZATION - Peripherals used
//==================================================================================

// ---------------------------------- USER -----------------------------------------
//  Put peripheral initialization here




//==================================================================================
//	INITIALIZATION - BUILD OPTIONS - Change in FlashingLeds-Settings.h
//==================================================================================

// ---------------------------------- USER -----------------------------------------

    Duty=_IQ24(0.0);
    Duty_Boost1=_IQ24(0.0);
    Duty_Boost2=_IQ24(0.0);
    ADC1Out=_IQ24(0.0);
    Dutyf=0.4;
    Duty_Boostf1=0.4;
    Duty_Boostf2=0.4;

    EALLOW;

    PWM_1ch_CNF(1,SYS_CLOCK/EPWM_FS,1,0);
    PWM_1ch_CNF(2,SYS_CLOCK/EPWM_FS,1,0);

	PWM_ComplPairDB_CNF(6,SYS_CLOCK/EPWM_FS, 1, 0);
	PWM_ComplPairDB_CNF(7,SYS_CLOCK/EPWM_FS, 0, 0);

	PWM_ComplPairDB_UpdateDB(6,100,100);
	PWM_ComplPairDB_UpdateDB(7,100,100);


	// Specify ADC Channel – pin Selection for Configuring the ADC
	ChSel[1] = 1; // ADC A5

	// Specify the Conversion Trigger for each channel
//	TrigSel[0]= ADCTRIG_EPWM1_SOCA;
	TrigSel[1]= ADCTRIG_EPWM1_SOCA;
	// Call the ADC Configuration Function
	ADC_SOC_CNF(ChSel,TrigSel,ACQPS,2,0);


	DPL_Init();

	PWMDRV_1ch_Duty1     = &Duty_Boost1;
	PWMDRV_1ch_Duty2     = &Duty_Boost2;
	PWMDRV_ComplPairDB_Duty6 = &Duty;
	PWMDRV_ComplPairDB_Duty7 = &Duty;

	ADCDRV_1ch_Rlt1 = &ADC1Out;
/*
	//EALLOW;
    EPwm1Regs.TZFRC.bit.OST = 1;
    EPwm2Regs.TZFRC.bit.OST = 1;
    EPwm6Regs.TZFRC.bit.OST = 1;
    EPwm7Regs.TZFRC.bit.OST = 1;

*/


#if(SPLL_SOGI_==1)
	SPLL_1ph_SOGI_IQ_init(50,_IQ18((float)(1.0/EPWM_FS)),&spll1);                                //SOGI锁相环
	SPLL_1ph_SOGI_IQ_coeff_update(((float)(1.0/EPWM_FS)),((float)(2*PI*50.0)),&spll1);            //
#endif
#if(SPLL_SOGI_==0)
	SPLL_1ph_IQ_init(50.0,_IQ21((float)(1.0/EPWM_FS)),&spll1);                                                      //锁相环
	SPLL_1ph_IQ_notch_coeff_update(((float)(1.0/EPWM_FS)),(float)(2*PI*50*2),(float)0.00001,(float)0.1,&spll1);     //
#endif

	EDIS;

//------------------------------------------------------
#if (INCR_BUILD == 1)
//------------------------------------------------------
	
	LedBlinkTimer = 0;			//Initialize LedBlinkTimer to 0
	Gui_LedPrd_ms = 1000;		//Default to 1 blink every second

#endif // (INCR_BUILD == 1)




//==================================================================================
//	INITIALIZATION - ISR - best to do this just before the infinite loop
//==================================================================================
// Initialize CLA if needed
//	CLA_Init();

// ---------------------------------- USER -----------------------------------------

    EALLOW;                                     //禁用所有中断
    PieVectTable.EPWM1_INT = &DPL_ISR;          // Map Interrupt
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;          // PIE level enable, Grp3 / Int1
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;    // INT on PRD event
    EPwm1Regs.ETSEL.bit.INTEN = 1;              // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;         // Generate INT on every event


    EPwm1Regs.ETSEL.bit.SOCASEL = 4;
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;

    PieVectTable.ADCINT1 = &ADC_INT;          // Map Interrupt
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
    AdcRegs.INTSEL1N2.bit.INT1SEL = 1;
    AdcRegs.INTSEL1N2.bit.INT1E =  1;
    AdcRegs.INTSEL1N2.bit.INT1CONT = 1;


    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
     PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1=1;     // PIE Group 9, INT1


//    PieVectTable.EPWM1_TZINT = &epwm1_tzint_isr;

    IER |= M_INT3;                              // Enable CPU INT3 connected to EPWM1-6 INTs:
    IER |= M_INT1;                              // Enable CPU INT1 connected to ADC
    IER |= M_INT9;                              // Enable CPU INT9 connected to SCI
    EINT;                                       // Enable Global interrupt INTM
    ERTM;                                       // Enable Global realtime interrupt DBGM
    EDIS;

//=================================================================================
//	BACKGROUND (BG) LOOP
//=================================================================================

//--------------------------------- FRAMEWORK -------------------------------------
	for(;;)  //infinite loop
	{
		// State machine entry & exit point
	 //   Duty=_IQ24(0.5);



		//===========================================================
		(*Alpha_State_Ptr)();	// jump to an Alpha state (A0,B0,...)
		//===========================================================


	}
} //END MAIN CODE



//=================================================================================
//	STATE-MACHINE SEQUENCING AND SYNCRONIZATION
//=================================================================================

//--------------------------------- FRAMEWORK -------------------------------------
void A0(void)
{
	// loop rate synchronizer for A-tasks
	if(CpuTimer0Regs.TCR.bit.TIF == 1)
	{
		CpuTimer0Regs.TCR.bit.TIF = 1;	// clear flag

		//-----------------------------------------------------------
		(*A_Task_Ptr)();// jump to an A Task (A1,A2,A3,...)
		//-----------------------------------------------------------


		SerialCommsTimer++;
	}

	Alpha_State_Ptr = &B0;		// Comment out to allow only A tasks
}

void B0(void)
{
	// loop rate synchronizer for B-tasks
	if(CpuTimer1Regs.TCR.bit.TIF == 1)
	{
		CpuTimer1Regs.TCR.bit.TIF = 1;				// clear flag

		//-----------------------------------------------------------
		(*B_Task_Ptr)();		// jump to a B Task (B1,B2,B3,...)
		//-----------------------------------------------------------
	}

	Alpha_State_Ptr = &C0;		// Allow C state tasks
}

void C0(void)
{
	// loop rate synchronizer for C-tasks
	if(CpuTimer2Regs.TCR.bit.TIF == 1)
	{
		CpuTimer2Regs.TCR.bit.TIF = 1;				// clear flag

		//-----------------------------------------------------------
		(*C_Task_Ptr)();		// jump to a C Task (C1,C2,C3,...)
		//-----------------------------------------------------------
	}

	Alpha_State_Ptr = &A0;	// Back to State A0
}


//----------------------------------- USER ----------------------------------------

//=================================================================================
//	A - TASKS
//=================================================================================
// Each active task runs every (CpuTimer0 period) * (# of active A Tasks)
//--------------------------------------------------------
void A1(void) // SCI-GUI
//--------------------------------------------------------
{
//	SerialHostComms();		// Serialport controls LED2 (GPIO-31)
				            // Will not blink until GUI is connected
    if(WaveSend_Button==1)
    {
        if(ADC_Send_flag==1)
        {
            ADC_Send_flag=0;

                scia_msg(Frame_head2);
                scia_send_float(ADC_Data[0]);
                scia_send_float(ADC_Data[1]);
                scia_msg(Frame_end);
        }
    }

    if(RX_OKflg == 1){
        GetRX_Cmd();
        RX_OKflg = 0;
    }



	if(LedBlinkTimer == 0)
	{
		GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;		// toggle GPIO34 which controls LD3 on most controlCARDs
		#if DSP2834x_DEVICE_H
		GpioDataRegs.GPBTOGGLE.bit.GPIO54 = 1;		// toggle GPIO54 which controls LD3 on the C2834x DIMM100 controlCARD
		#endif
//		Duty_Boost1=_IQ24(0.5);
		LedBlinkTimer = Gui_LedPrd_ms >> 1;			// divide by 2 (one LED blink period is 2 LED toggles)
	}
	else
	{
		LedBlinkTimer--;
	}
	

	//-------------------
	//the next time CpuTimer0 'counter' reaches Period value go to A1
	A_Task_Ptr = &A1;
	//-------------------
}

//--------------------------------
void A2(void) //  SPARE
//--------------------------------
{	

	//-------------------
	//task never runs; Task A1 always never set A_Task_Ptr to come to A2
	A_Task_Ptr = &A1;
	//-------------------
}


//=================================================================================
//	B - TASKS
//=================================================================================
// Each active task runs every (CpuTimer1 period) * (# of active B Tasks)
//--------------------------------
void B1(void) //  SPARE
//--------------------------------
{


if(Run_Button==0)
{

    EPwm6Regs.AQCSFRC.bit.CSFA=0x02;
    EPwm6Regs.AQCSFRC.bit.CSFB=0x02;
    EPwm6Regs.DBCTL.bit.OUT_MODE = 0;

    EPwm7Regs.AQCSFRC.bit.CSFA=0x02;
    EPwm7Regs.AQCSFRC.bit.CSFB=0x02;
    EPwm7Regs.DBCTL.bit.OUT_MODE = 0;

    EPwm1Regs.AQCSFRC.bit.CSFA=0x02;
    EPwm2Regs.AQCSFRC.bit.CSFA=0x02;

}else if(Run_Button==1 && EPwm6Regs.DBCTL.bit.OUT_MODE == 0)
{
    EPwm6Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm6Regs.AQCSFRC.bit.CSFA=0x00;
    EPwm6Regs.AQCSFRC.bit.CSFB=0x00;

    EPwm7Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm7Regs.AQCSFRC.bit.CSFA=0x00;
    EPwm7Regs.AQCSFRC.bit.CSFB=0x00;

    EPwm1Regs.AQCSFRC.bit.CSFA=0x00;
    EPwm2Regs.AQCSFRC.bit.CSFA=0x00;
}






	//-----------------
	//the next time CpuTimer1 'counter' reaches Period value go to B2
	B_Task_Ptr = &B2;	
	//-----------------
}

//--------------------------------
void B2(void) //  SPARE
//--------------------------------
{

	//-----------------
	//the next time CpuTimer1 'counter' reaches Period value go to B1
	B_Task_Ptr = &B1;
	//-----------------
}


//=================================================================================
//	C - TASKS
//=================================================================================
	// Each active task runs every (CpuTimer2 period) * (# of active C Tasks)
//--------------------------------
void C1(void) 	// SPARE
//--------------------------------
{

//    EALLOW;

 //   EDIS;



//    EPwm1Regs.TZSEL.bit.OSHT2 = 1;


	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C2
	C_Task_Ptr = &C2;	
	//-----------------

}

//--------------------------------
void C2(void) //  SPARE
//--------------------------------
{

	//-----------------
	//the next time CpuTimer2 'counter' reaches Period value go to C1
	C_Task_Ptr = &C1;	
	//-----------------
}

__interrupt void ADC_INT(){

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 //****输入信号 要去掉直流分量
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


/****************************************************
 * 锁相环切换
 */
#if(SPLL_SOGI_==0)
    spll1.AC_input =(long)(_IQ21mpy((ADC1Out)>>3,_IQ21(1)));
    SPLL_1ph_IQ_FUNC(&spll1);
    if(ADC_Send_flag==0)
    {

 //    ADC_Data[0]=_IQ21toF(ADC1Out);
        ADC_Data[1] = _IQ21toF(spll1.wo);
        ADC_Data[0] = _IQ21toF(spll1.sin[0]);
        ADC_Send_flag = 1;
    }

    if(spll1.sin[0]>0)
    {
        GpioDataRegs.GPASET.bit.GPIO6 = 1;
    }else GpioDataRegs.GPADAT.bit.GPIO6 = 0;


#endif
#if(SPLL_SOGI_==1)

    spll1.u[0]=(long)(_IQ23mpy((ADC1Out-_IQ24(0.45))>>1,_IQ23(2.12)));
    SPLL_1ph_SOGI_IQ_FUNC(&spll1);
    if(ADC_Send_flag==0)
       {

       //    ADC_Data[0]=_IQ21toF(ADC1Out);
           ADC_Data[1] = _IQ23toF(spll1.fn);
           ADC_Data[0] = _IQ23toF(spll1.u_Q[0]);

           ADC_Send_flag = 1;
       }
       if(spll1.sin>0)
       {
           GpioDataRegs.GPASET.bit.GPIO6 = 1;
       }else GpioDataRegs.GPADAT.bit.GPIO6 = 0;

       ADC_Data[3]=_IQ23toF(spll1.theta[0]);

#endif





    Duty = _IQ24(Dutyf);
    Duty_Boost1 = _IQ24(Duty_Boostf1);
    Duty_Boost2 = _IQ24(Duty_Boostf2);


    AdcRegs.ADCINTFLG.bit.ADCINT1 = 0;      // clear interrupt flag for ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT = 0;     // clear ADCINT1 flag to begin a new set of conversions
}


