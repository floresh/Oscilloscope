#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "lm3s2110.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/comp.h"
#include "driverlib/timer.h"
#include "network.h"


#define PERIOD_CLOCK 10


void compConfig(void);
void Timer0A_ISR(void);
void Timer0B_ISR(void);
void TimersConfig(void);


volatile int count = 0;
volatile int accum_period = 0;
volatile int resetValues = 0;

volatile unsigned long period = 0;
volatile unsigned long g_ulSystemClock;
/*
 * main.c
 */
int main(void)
{
	if(REVISION_IS_A2)
		SysCtlLDOSet(SYSCTL_LDO_2_75V);
	SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
					SYSCTL_XTAL_8MHZ);
	g_ulSystemClock = SysCtlClockGet();
	
	compConfig();
	TimersConfig();
	NetworkInit();

	while(1)
	{
	}
}



void compConfig(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
	ComparatorConfigure(COMP_BASE, 0, COMP_TRIG_RISE | COMP_ASRCP_REF);
	ComparatorRefSet(COMP_BASE, COMP_REF_1_5125V);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_DIR_MODE_HW); //C0 - input
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_ANALOG);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_DIR_MODE_HW); //C0o output
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
}



void TimersConfig(void)
{
	unsigned long ulDivider, ulPrescaler;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_DIR_MODE_HW); // CCP0 input
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

	TimerDisable(TIMER0_BASE, TIMER_BOTH);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_PERIODIC);

	/* Timer A control event and load value */
	TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
	TimerLoadSet(TIMER0_BASE, TIMER_A, 0xffff);


	/* Timer B prescaler and load set*/
	//PRESCALER FOR 16-BIT TIMER B
	ulPrescaler = (g_ulSystemClock / PERIOD_CLOCK - 1) >> 16;
	//16-bit divider (timer load value)
	ulDivider = g_ulSystemClock / (PERIOD_CLOCK * (ulPrescaler + 1)) - 1;
	TimerLoadSet(TIMER0_BASE, TIMER_B, ulDivider);
	TimerPrescaleSet(TIMER0_BASE, TIMER_B, ulPrescaler);


	/* enable timers and their interrupts */
	TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);//timer A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);//timer B
	TimerEnable(TIMER0_BASE, TIMER_BOTH);

	//intialize interrupt controller to respond to timer intrrupts
	IntPrioritySet(INT_TIMER0A, 0); //highest priority, 32 = next lower
	IntPrioritySet(INT_TIMER0B, 32);
	IntEnable(INT_TIMER0A);
	IntEnable(INT_TIMER0B);
}


void Timer0A_ISR(void)
{
	TIMER0_ICR_R = TIMER_ICR_CAECINT; //| TIMER_ICR_TATOCINT;//clear interrupt flag

	static int last_capture = 0;
	int capture = TIMER0_TAR_R ;

	if(resetValues)
	{
		count = 0;
		accum_period = 0;
		resetValues = 0;
	}

	accum_period += (last_capture - capture) & 0xffff;

	last_capture = capture;
	count++;
}



void Timer0B_ISR(void)
{
	TIMER0_ICR_R = TIMER_ICR_TBTOCINT;//clear timeout interrupt flag

	int local_count = count;
	unsigned long frequency;
	period = accum_period;

	resetValues = 1;

	period = (period/count);
	frequency = 1/(period * (0.00000004))*1000;

	NetworkTx(frequency);
}









