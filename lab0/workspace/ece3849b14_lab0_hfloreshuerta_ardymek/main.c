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


#define BUTTON_CLOCK 200 //button scanning interrupt rate in Hz

unsigned long g_ulSystemClock; //system clock frequency in Hz
volatile unsigned long g_ulTime = 0; //time in hundreths of a second


void TimerISR(void);
void timersInit(void);
void switchesInit(void);

int main(void)
{
	char pcStr[50]; //string buffer
	unsigned long ulTime; //local copy of g_ulTime
	unsigned long milliseconds,seconds, minutes;

	//initialize system clock to 50 MHz
	if(REVISION_IS_A2)
		SysCtlLDOSet(SYSCTL_LDO_2_75V);

	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
	g_ulSystemClock = SysCtlClockGet();
	
	RIT128x96x4Init(3500000); //initialize the OLED display

	switchesInit();
	timersInit();

	while(true)
	{
		FillFrame(0); //clear frame buffer
		ulTime = g_ulTime; //read volatile global vriable only once

		milliseconds = ulTime %60;
		seconds = (ulTime / 100) % 60;
		minutes = (ulTime / 6000);

		usprintf(pcStr, "Time = %02d:%02d:%02d", minutes,seconds,milliseconds); //convert time to string
		DrawString(0, 0, pcStr, 15, false); //draw string to frame buffer
		RIT128x96x4ImageDraw(g_pucFrame, 0, 0, FRAME_SIZE_X, FRAME_SIZE_Y); //copy frame to the OLED screen
	}
}

//configure GPIO used to read the state of the on-board push buttons
void switchesInit(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

void timersInit(void)
{
	unsigned long ulDivider, ulPrescaler;

	//initialize a general purpose timer for periodix interrupts
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerDisable(TIMER0_BASE, TIMER_BOTH);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);

	//PRESCALER FOR 16-BIT TIMER
	ulPrescaler = (g_ulSystemClock / BUTTON_CLOCK - 1) >> 16;
	//16-bit divider (timer load value)
	ulDivider = g_ulSystemClock / (BUTTON_CLOCK * (ulPrescaler + 1)) - 1;

	TimerLoadSet(TIMER0_BASE, TIMER_A, ulDivider);
	TimerPrescaleSet(TIMER0_BASE, TIMER_A, ulPrescaler);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A);

	//intialize interrupt controller to respond to timer intrrupts
	IntPrioritySet(INT_TIMER0A, 0); //highest priority, 32 = next lower
	IntEnable(INT_TIMER0A);
	IntMasterEnable();
}

void TimerISR(void)
{
	static int tic = false;
	static int running = true;
	unsigned long presses = g_ulButtons;

	TIMER0_ICR_R = TIMER_ICR_TATOCINT; //clear interrupt flag
	ButtonDebounce( ((GPIO_PORTF_DATA_R & GPIO_PIN_1) >> 1) | ((GPIO_PORTE_DATA_R & (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)) << 1) );
	presses = ~presses & g_ulButtons; //button press detector

	if(presses & 1)
		running = !running;
	if(presses & 0x1E)
		g_ulTime = 0;

	if(running)
	{
		if(tic)
		{
			g_ulTime++; //increment time every other ISR call
			tic = false;
		}
		else
			tic = true;
	}
}




//






