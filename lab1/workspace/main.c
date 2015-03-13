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

#define SELECT 1
#define UP 2
#define DOWN 4
#define LEFT 8
#define RIGHT 16

#define BUTTON_CLOCK 200 //button scanning interrupt rate in Hz
#define FIFO_SIZE 10//button reading FIFO size

#define TRIGGER_LOOPS 956//amount of times to loop before giving up on finding trigger

#define ADC_OFFSET 515
#define VIN_RANGE 6
#define PIXELS_PER_DIV 12
#define ADC_BITS 1024

#define ADC_BUFFER_SIZE 2048 // must be a power of 2
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping macro

#define ADCBUFFERSIZE_LOCAL 128//Frame ADC buffer size

unsigned long g_ulSystemClock; //system clock frequency in Hz
volatile unsigned long g_ulTime = 0; //time in hundreths of a second

volatile int g_iADCBufferIndex = ADC_BUFFER_SIZE - 1; // latest sample index
volatile unsigned short g_pusADCBuffer[ADC_BUFFER_SIZE]; // circular buffer
volatile unsigned long g_ulADCErrors = 0; // number of missed ADC deadlines

volatile unsigned char buttonFIFO[10];
volatile unsigned int headFIFO = 0;
volatile unsigned int tailFIFO = 0;

void drawSlope(int slope);
void drawGrid(void);
int getButtons(void);
void putButtons(int presses);
void ADCInit(void);
void ADC_ISR(void);
void TimerISR(void);
void timersInit(void);
void switchesInit(void);
unsigned long cpu_load_count(void);




int main(void)
{
	//cpu load calulation cariables
	int count_unloaded;
	int count_loaded;
	float cpu_load;

	int cpu_load_integer;
	int cpu_load_decimal;
	char pcStr[50]; //string buffer
	int cpu_timer = 0;//recalculate CPU load every 100 screen draws

	int trigger_count;//give trigger 100 cycles to readjust before drawing non-triggered signal

	volatile int i;
	volatile int y_coordinate;
	volatile int lasty_coordinate;

	volatile unsigned short ADCBuffer[128]; //local circular buffer
	volatile unsigned short ADCSample;
	volatile unsigned short ADCSample_next;

	volatile unsigned short trigger;//index value of trigger in global buffer
	volatile int ADCBufferIndex = 0;

	volatile int notDone = TRIGGER_LOOPS;//amount of times to loop before giving up on trigger
	volatile int triggered = 0;// set to 1 when trigger is found
	volatile int triggerRange;//to allow trigger to be a small range of values

	volatile int slope = 0;//selects slope to draw; 0=falling_edge, 1=rising_edge
	volatile int triggerSlope;

	volatile int buttonPressed;

	volatile int editingValue = 0;//0=voltage_scale, 1=time_scale
	volatile int editValue = 0;//[0,3] -> see voltageScaleStr
	volatile int voltageScaleSelect = 2;//selects voltage scale; = [0,3] -> see fVoltageScale
	const char * const voltageScaleStr[] = {"100 mV", "200 mV", "500 mV", "1 V"};

	float fVoltageScale[4] =  {0.1,0.2,0.5,1};
	volatile float fVoltsPerDiv = 0.5;
	volatile float fScale = 5*(VIN_RANGE * PIXELS_PER_DIV)/(ADC_BITS * fVoltsPerDiv);


//	char pcStr[50]; //string buffer

	//initialize system clock to 50 MHz
	if(REVISION_IS_A2)
		SysCtlLDOSet(SYSCTL_LDO_2_75V);

	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
	g_ulSystemClock = SysCtlClockGet();

	RIT128x96x4Init(3500000); //initialize the OLED display
	switchesInit();//initialize switches
	timersInit();//initialize timers
	ADCInit();//initialize ADC

	//////////////////////////////////////////////////////////////////////////////
	// code for keeping track of CPU load
	// initialize timer 3 in one-shot mode for polled timing
	IntMasterDisable();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	TimerDisable(TIMER3_BASE, TIMER_BOTH);
	TimerConfigure(TIMER3_BASE, TIMER_CFG_ONE_SHOT);
	/*
	 * Clock runs at 50MHz so 50,000,000 counts = 50,000,000/(50MHz) = 1s
	 * therefore 1,000,000/(50MHz) = 20ms
	 */
	TimerLoadSet(TIMER3_BASE, TIMER_A, 1000000); // 20ms interval

	count_unloaded = cpu_load_count();

	IntMasterEnable();

	count_loaded = cpu_load_count();
	cpu_load = 1.0 - (float)count_loaded/count_unloaded; // compute CPU load
	cpu_load_integer = cpu_load * 100;
	cpu_load_decimal = (int)((cpu_load * 10000)) % 100;

	/*
	while (1) {
		count_loaded = cpu_load_count();
		cpu_load = 1.0 - (float)count_loaded/count_unloaded; // compute CPU load
	}
	*/


	while(1)
	{
		/*find trgger*/
		ADCBufferIndex = ADC_BUFFER_WRAP(g_iADCBufferIndex - 68);//start one screen width behind current buffer index
		ADCSample = g_pusADCBuffer[ADCBufferIndex];							 //aquire sample
		ADCSample_next = g_pusADCBuffer[ADC_BUFFER_WRAP(ADCBufferIndex - 4)];//aquire next sample to compare current sample to

		while(notDone)
		{
			notDone--;//if we have yet to traverse half the global ADC buffer
			triggerRange = ADCSample - ADC_OFFSET;//trigger offset
			triggerSlope = (slope) ? (ADCSample_next < ADCSample) : (ADCSample_next > ADCSample);//set trigger slope

			if(triggerSlope && (triggerRange > -2 && triggerRange < 2))
			{
				trigger = ADCBufferIndex;
				triggered = 1;
				notDone = 0;
			}

			ADCBufferIndex  = ADC_BUFFER_WRAP(ADCBufferIndex - 1);//get next sample index
			ADCSample = g_pusADCBuffer[ADCBufferIndex];			//aquire sample
			ADCSample_next = g_pusADCBuffer[ADC_BUFFER_WRAP(ADCBufferIndex - 1)];//aquire next sample
		}


		if(!triggered)
			trigger_count++;

		notDone = TRIGGER_LOOPS;//reset loop value


		/*proceed to handle drawing next frame*/
		if(triggered | (trigger_count >= 10))
		{
			/*
			 * if triggered start half a screen width behind trigger
			 * else if not triggered start drawing a
			 * screen width begind current index position
			 */
			trigger = (triggered) ? (trigger -64) : (g_iADCBufferIndex-128);
			trigger_count = 0;//reset trigger count
			triggered = 0;//reset trigger

			/*read in global ADC buffer to local buffer*/
			for(i=0; i<128; i++)
			{
				ADCBuffer[ADC_BUFFER_WRAP(i)] = g_pusADCBuffer[ADC_BUFFER_WRAP(trigger + i)];
			}

			FillFrame(0); //clear frame buffer
			drawGrid();	  //12x12 pixel draw grid lines

			/*draw signal to screen*/
			//draw first point to get reference for first line
			ADCSample = ADCBuffer[0];
			y_coordinate = 48 - (int)(((int)ADCSample - ADC_OFFSET) * fScale);
			lasty_coordinate = y_coordinate;
			DrawPoint(i, y_coordinate, 15);

			// loop through local buffer to draw waveform
			for(i=0; i < 128; i++)
			{
				ADCSample = ADCBuffer[i];
				y_coordinate = 48 - (int)(((int)ADCSample - ADC_OFFSET) * fScale);//get y coordinate according to sample scaling formula

				DrawLine(i-1,lasty_coordinate, i,y_coordinate,15);//draw line between last point and current point

				lasty_coordinate = y_coordinate;
				DrawPoint(i, y_coordinate, 15);//draw current point
			}

			/*draw waveform information*/
			//draw trigger slope
			drawSlope(slope);

			//draw line under value being edited
			if(editingValue)//if button was pressed to edit value
			{
				voltageScaleSelect = (voltageScaleSelect + editValue) % 4;//edit voltage scale selector
				editValue = 0;//reset redit value

				fVoltsPerDiv = fVoltageScale[voltageScaleSelect];//select voltage per division
				fScale = 6*(VIN_RANGE * PIXELS_PER_DIV)/(ADC_BITS * fVoltsPerDiv);//recalculate voltage scale

				DrawFilledRectangle(44,7,80,0,8);
			}
			else
				DrawFilledRectangle(4,7,34,0,8);

			/*recompute CPU Load*/
			cpu_timer++;
			if(cpu_timer == 100)
			{
				cpu_timer = 0;
				count_loaded = cpu_load_count();
				cpu_load = 1.0 - (float)count_loaded/count_unloaded; // compute CPU load
				cpu_load_integer = cpu_load * 100;//calculate cpu load integer value
				cpu_load_decimal = (int)((cpu_load * 10000)) % 100;//calculate cpu load fractional value
			}

			DrawString(4, 0, "24 us", 15, false);
			DrawString(45, 0, voltageScaleStr[voltageScaleSelect], 15, false);
			usprintf(pcStr, "CPU Load = %d.%d %%", cpu_load_integer,cpu_load_decimal); //convert cpu_load to string
			DrawString(0, 84, pcStr, 15, false); //draw CPU load string to frame buffer

			RIT128x96x4ImageDraw(g_pucFrame, 0, 0, FRAME_SIZE_X, FRAME_SIZE_Y); //copy frame to the OLED screen
		}


		/* handle FIFO for reading in button presses*/

		if(headFIFO != tailFIFO)//if FIFO not empty
		{
			buttonPressed = getButtons();

			if(buttonPressed & SELECT)//select button was hit
				slope ^= 1;

			else if(buttonPressed & UP)//up button
				editValue = 1;//set scroll value to +1
			else if(buttonPressed & DOWN)
			{
				editValue = 3;
			}

			else if(buttonPressed & LEFT)
			{
				editingValue ^= 1;//toggle editing value
				editValue = 0;//reset edit amount
			}
			else if(buttonPressed & RIGHT)
			{
				editingValue ^= 1;//toggle editing value
				editValue = 0;//reset editing amount
			}

		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	IntPrioritySet(INT_ADC0SS0,0); // set ADC interrupt priority in the interrupt controller
	IntEnable(INT_ADC0SS0); // enable ADC interrupt
}

void ADC_ISR(void)
{
//	ADCIntClear(ADC0_BASE,0);
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

	//initialize a general purpose timer for periodic interrupts
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
	IntPrioritySet(INT_TIMER0A, 32); //highest priority, 32 = next lower
	IntEnable(INT_TIMER0A);
	IntMasterEnable();
}

void TimerISR(void)
{
	unsigned long presses = g_ulButtons;

	TIMER0_ICR_R = TIMER_ICR_TATOCINT; //clear interrupt flag
	//bit 0 = select,1 = up,2 = down,3 = left,4 = right
	ButtonDebounce( ((GPIO_PORTF_DATA_R & GPIO_PIN_1) >> 1) | ((GPIO_PORTE_DATA_R & (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)) << 1) );
	presses = ~presses & g_ulButtons; //button press detector

	if(presses)
		putButtons(presses);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long cpu_load_count(void)
{
	unsigned long i = 0;
	TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER3_BASE, TIMER_A); // start one-shot timer
	while (!(TimerIntStatus(TIMER3_BASE, 0) & TIMER_TIMA_TIMEOUT))
		i++;
	return i;
}

int getButtons(void)
{
	int buttonPressed = buttonFIFO[headFIFO];//read in button press

	//readjust FIFO head pointer
	if(headFIFO + 1 >= FIFO_SIZE)
		headFIFO = 0;
	else
		headFIFO++;

	return buttonPressed;
}

void putButtons(int presses)
{
	int newTail = tailFIFO + 1;

	//wrap tail pointer
	if(newTail >= FIFO_SIZE )
		newTail = 0;

	//if FIFO is not full
	if(newTail != headFIFO)
	{//fill FIFO with current button presses
		buttonFIFO[tailFIFO] = presses;
		tailFIFO = newTail;
	}
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







