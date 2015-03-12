/*
 *  ======== main.c ========
 */
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/lm3s8962.h"
#include "driverlib/sysctl.h"

#include "drivers/rit128x96x4.h"
#include "frame_graphics.h"
#include "utils/ustdlib.h"

#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "driverlib/gpio.h"
#include "buttons.h"

#include "driverlib/adc.h"

#include <xdc/std.h>
#include <xdc/cfg/global.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/sysbios/knl/Task.h>

#define SELECT 1
#define UP 2
#define DOWN 4
#define LEFT 8
#define RIGHT 16

#define ADC_BUFFER_SIZE 2048 // must be a power of 2
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping macro

#define ADCBUFFERSIZE_LOCAL 128//Frame ADC buffer size

unsigned long g_ulSystemClock; //system clock frequency in Hz
volatile unsigned long g_ulTime = 0; //time in hundreths of a second

volatile int g_iADCBufferIndex = ADC_BUFFER_SIZE - 1; // latest sample index
volatile unsigned short g_pusADCBuffer[ADC_BUFFER_SIZE]; // circular buffer
volatile unsigned long g_ulADCErrors = 0; // number of missed ADC deadlines


void drawGrid(void);
void drawSlope(int slope);

/*
 *  ======== main ========
 */
Int main()
{
    BIOS_start();    /* does not return */
    return(0);
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//configure the ADC at 500,000 Hz

void ADCInit()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // enable the ADC
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_500KSPS); // specify 500ksps

	ADCSequenceDisable(ADC0_BASE, 0); // choose ADC sequence 0; disable before configuring
	ADCSequenceConfigure(ADC0_BASE,0,ADC_TRIGGER_ALWAYS,0); // specify the "Always" trigger
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END); // in the 0th step, sample channel 0
																				  // enable interrupt, and make it the end of sequence

	ADCIntEnable(ADC0_BASE, 0); // enable ADC interrupt from sequence 0
	ADCSequenceEnable(ADC0_BASE, 0); // enable the sequence. it is now sampling
//	IntPrioritySet(INT_ADC0SS0,0); // set ADC interrupt priority in the interrupt controller
	IntEnable(INT_ADC0SS0); // enable ADC interrupt
}


void ADC_Hwi(void)
{
	ADC0_ISC_R = 1;

	if (ADC0_OSTAT_R & ADC_OSTAT_OV0)
	{ // check for ADC FIFO overflow
		g_ulADCErrors++; // count errors - step 1 of the signoff
		ADC0_OSTAT_R = ADC_OSTAT_OV0; // clear overflow condition
	}

	int buffer_index = ADC_BUFFER_WRAP(g_iADCBufferIndex + 1);
	g_pusADCBuffer[buffer_index] = ADC_SSFIFO0_R; // read sample from the ADC sequence0 FIFO
	g_iADCBufferIndex = buffer_index;
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void buttonScanning(void)
{
	unsigned long presses = g_ulButtons;

	//bit 0 = select,1 = up,2 = down,3 = left,4 = right
	ButtonDebounce( ((GPIO_PORTF_DATA_R & GPIO_PIN_1) >> 1) | ((GPIO_PORTE_DATA_R & (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)) << 1) );
	presses = ~presses & g_ulButtons; //button press detector

	if(presses)
		Mailbox_post(buttonMailbox,&presses,BIOS_NO_WAIT);
}



void drawGrid()
{
	/*draw grid below signal*/
	DrawLine(0,12,128,12,2);
	DrawLine(0,24,128,24,2);
	DrawLine(0,36,128,36,2);

	DrawLine(0,48,128,48,8);

	DrawLine(0,60,128,60,2);
	DrawLine(0,72,128,72,2);
	DrawLine(0,84,128,84,2);
	DrawLine(0,96,128,96,2);


	DrawLine(4,0,4,96,1);
	DrawLine(16,0,16,96,1);
	DrawLine(28,0,28,96,1);
	DrawLine(40,0,40,96,1);
	DrawLine(52,0,52,96,1);
	DrawLine(64,0,64,96,1);
	DrawLine(76,0,76,96,1);
	DrawLine(88,0,88,96,1);
	DrawLine(100,0,100,96,1);
	DrawLine(112,0,112,96,1);
	DrawLine(124,0,124,96,1);
}

void drawSlope(int slope)
{
	if(slope)
	{
		DrawLine(92,10,100,10,15);
		DrawLine(100,2,108,2,15);
		DrawLine(100,10,100,2,15);
	}
	else
	{
		DrawLine(92,2,100,2,15);
		DrawLine(100,10,108,10,15);
		DrawLine(100,10,100,2,15);
	}

	return;
}

