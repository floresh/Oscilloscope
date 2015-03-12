#include "math.h"
#include "kiss_fft.h"
#include "_kiss_fft_guts.h"

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

#include "network.h"

#define PI 3.14159265358979
#define NFFT 1024 // FFT length
#define KISS_FFT_CFG_SIZE (sizeof(struct kiss_fft_state)+sizeof(kiss_fft_cpx)*(NFFT-1))
#define FFT_OFFSET 450

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

volatile unsigned short ADCBuffer[128]; //local circular buffer

/* Shared Oscilloscope settings */
volatile int slope = 0;//selects slope to draw; 0=falling_edge, 1=rising_edge

enum EDIT {TIME=0,VOLTAGE=1,SLOPE=2};
enum EDIT editingValue = TIME;//0=voltage_scale, 1=time_scale, 2=slope
volatile int editValue = 0;//[0,3] -> see voltageScaleStr

volatile int voltageScaleSelect = 2;//selects voltage scale; = [0,3] -> see fVoltageScale
volatile int displayFFT = 0;
volatile unsigned short trigger;//index value of trigger in global buffer

volatile unsigned long g_ulfrequency;

static kiss_fft_cpx in[NFFT];


void ADCInit(void);
void drawGrid(void);
void switchesInit(void);
void drawSlope(int slope);


/*
 *  ======== main ========
 */
Int main()
{
	RIT128x96x4Init(3500000); //initialize the OLED display
	switchesInit();
	NetworkInit();
	ADCInit();

    BIOS_start();    /* does not return */

    return(0);
}



void findTrigger(void)
{
	unsigned short ADCSample;
	unsigned short ADCSample_next;

	unsigned short trigger;//index value of trigger in global buffer
	int ADCBufferIndex = 0;

	int notDone = TRIGGER_LOOPS;//amount of times to loop before giving up on trigger
	int triggered = 0;// set to 1 when trigger is found
	int triggerRange;//to allow trigger to be a small range of values

	int triggerSlope;
	int triggerCount;
	int i;
	int index;


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
			triggerCount++;

		notDone = TRIGGER_LOOPS;//reset loop value


		/*proceed to handle drawing next frame*/
		if(triggered || (triggerCount >= 5))
		{
			trigger = (triggered) ? (trigger-64) : (g_iADCBufferIndex-128);

			triggerCount = 0;//reset trigger count
			triggered = 0;//reset trigger

			if(displayFFT)
			{
				/*read in global ADC buffer to local buffer*/
				index = g_iADCBufferIndex - 1024;
				for(i=0; i<NFFT; i++)
				{
					in[i].r = g_pusADCBuffer[ADC_BUFFER_WRAP(index + i)];
					in[i].i = 0;
				}


				Semaphore_post(FFTSemaphore);
				Semaphore_pend(waveSemaphore,BIOS_WAIT_FOREVER);
			}
			else
			{
				/*read in global ADC buffer to local buffer*/
				for(i=0; i<128; i++)
					ADCBuffer[ADC_BUFFER_WRAP(i)] = g_pusADCBuffer[ADC_BUFFER_WRAP(trigger + i)];


				Semaphore_post(displaySemaphore);//post to display semaphore
				Semaphore_pend(waveSemaphore,BIOS_WAIT_FOREVER);
			}
		}
	}
}



void drawFrame(void)
{
	unsigned short ADCSample;

	int i;
	int y_coordinate;
	int lasty_coordinate;

	char pcStr[50]; //string buffer

	float fVoltsPerDiv = 0.5;
	float fVoltageScale[4] =  {0.1,0.2,0.5,1};
	const char * const voltageScaleStr[] = {"100 mV", "200 mV", "500 mV", "1 V"};
	volatile float fScale = 5*(VIN_RANGE * PIXELS_PER_DIV)/(ADC_BITS * fVoltsPerDiv);

	unsigned long frequency;
	unsigned long frequency_dp;



	while(1)
	{
		Semaphore_pend(displaySemaphore, BIOS_WAIT_FOREVER);

		FillFrame(0); //clear frame buffer
		drawGrid();	  //12x12 pixel draw grid lines


		/*draw signal to screen*/
		//draw first point to get reference for first line
		ADCSample = ADCBuffer[0];
		y_coordinate = 48 - (int)(((int)ADCSample - ADC_OFFSET) * fScale);
		lasty_coordinate = y_coordinate;
		DrawPoint(i, y_coordinate, 15);

		for(i=0; i < 128; i++)
		{
			ADCSample = ADCBuffer[i];
			y_coordinate = 48 - (int)(((int)ADCSample - ADC_OFFSET) * fScale);//get y coordinate according to sample scaling formula

			DrawLine(i-1,lasty_coordinate, i,y_coordinate,15);//draw line between last point and current point

			lasty_coordinate = y_coordinate;
			DrawPoint(i, y_coordinate, 15);//draw current point
		}


		/*draw waveform information*/
		//draw line under value being edited
		switch(editingValue)
		{
		case VOLTAGE:
			voltageScaleSelect = (voltageScaleSelect + editValue) % 4;//edit voltage scale selector
			editValue = 0;//reset redit value

			fVoltsPerDiv = fVoltageScale[voltageScaleSelect];//select voltage per division
			fScale = 6*(VIN_RANGE * PIXELS_PER_DIV)/(ADC_BITS * fVoltsPerDiv);//recalculate voltage scale

			DrawFilledRectangle(44,7,80,0,8);
			break;

		case TIME:
			DrawFilledRectangle(4,7,34,0,8);
			break;

		case SLOPE:
			DrawFilledRectangle(92,2,108,10,8);
			break;
		}


		frequency = g_ulfrequency;
		frequency_dp = frequency%1000;
		frequency /= 1000;
		usprintf(pcStr, "Frequency = %d.%d",frequency,frequency_dp); //convert frequency to string


		drawSlope(slope);//draw trigger slope
		DrawString(4, 0, "24 us", 15, false);
		DrawString(45, 0, voltageScaleStr[voltageScaleSelect], 15, false);
		DrawString(0, 84, pcStr, 15, false); //draw frequency string to frame buffer

		RIT128x96x4ImageDraw(g_pucFrame, 0, 0, FRAME_SIZE_X, FRAME_SIZE_Y); //copy frame to the OLED screen

		if(displayFFT)
		{
			fVoltsPerDiv = fVoltageScale[2];//select voltage per division
			fScale = 6*(VIN_RANGE * PIXELS_PER_DIV)/(ADC_BITS * fVoltsPerDiv);//recalculate voltage scale
			Semaphore_post(FFTSemaphore);
		}
		else
			Semaphore_post(waveSemaphore);
	}
}

void buttonHandler(void)
{
	char buttonPressed = 0;

	while(1)
	{
		Mailbox_pend(buttonMailbox, (xdc_Ptr)(&buttonPressed), BIOS_WAIT_FOREVER);

		switch(buttonPressed)
		{
		case SELECT:
			if(editingValue == SLOPE)
				slope ^= 1;
			else
				displayFFT ^= 1;
			break;

		case UP:
			editValue = 1;
			break;

		case DOWN:
			editValue = 3;
			break;

		case LEFT:
			editingValue = (editingValue + 2) % 3;//toggle editing value
			editValue = 0;//reset edit amount
			break;

		case RIGHT:
			editingValue = (editingValue + 1) % 3;//toggle editing value
			editValue = 0;//reset edit amount
			break;

		default:
			break;
		}
	}
}


void buttonScanning(void)
{
	char presses = g_ulButtons;

	//bit 0 = select,1 = up,2 = down,3 = left,4 = right
	ButtonDebounce( ((GPIO_PORTF_DATA_R & GPIO_PIN_1) >> 1) | ((GPIO_PORTE_DATA_R & (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)) << 1) );
	presses = ~presses & g_ulButtons; //button press detector

	if(presses)
		Mailbox_post(buttonMailbox, (xdc_Ptr)(&presses), BIOS_NO_WAIT);
}



void calculateFFT(void)
{
	static char kiss_fft_cfg_buffer[KISS_FFT_CFG_SIZE]; // Kiss FFT config memory
	size_t buffer_size = KISS_FFT_CFG_SIZE;
	kiss_fft_cfg cfg; // Kiss FFT config

	static kiss_fft_cpx out[NFFT]; // complex waveform and spectrum buffers
	int i;
	cfg = kiss_fft_alloc(NFFT, 0, kiss_fft_cfg_buffer, &buffer_size); // init Kiss FFT

	while(1)
	{
		Semaphore_pend(FFTSemaphore, BIOS_WAIT_FOREVER);
		kiss_fft(cfg, in, out); // compute FFT

		for(i = 0; i < 128; i++)
		{
			ADCBuffer[i] = 10*log10( ( out[i].i*out[i].i ) + ( out[i].r*out[i].r ) ) + FFT_OFFSET;
		}

		Semaphore_post(displaySemaphore);
		Semaphore_post(waveSemaphore);
	}
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
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH0|ADC_CTL_IE); // in the 3rd & 7th step, sample channel 0
	ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END);// enable interrupt, and make it the end of sequence

	ADCIntEnable(ADC0_BASE, 0); // enable ADC interrupt from sequence 0
	ADCSequenceEnable(ADC0_BASE, 0); // enable the sequence. it is now sampling
//	IntPrioritySet(INT_ADC0SS0,0); // set ADC interrupt priority in the interrupt controller
	IntEnable(INT_ADC0SS0); // enable ADC interrupt
}

void ADC_Hwi(void)
{
	ADC0_ISC_R = 1;
	int buffer_index;

	if (ADC0_OSTAT_R & ADC_OSTAT_OV0)
	{ // check for ADC FIFO overflow
		g_ulADCErrors++; // count errors - step 1 of the signoff
		ADC0_OSTAT_R = ADC_OSTAT_OV0; // clear overflow condition
	}

	while(!(ADC0_SSFSTAT0_R & ADC_SSFSTAT1_EMPTY))
	{
		buffer_index = ADC_BUFFER_WRAP(g_iADCBufferIndex + 1);
		g_pusADCBuffer[buffer_index] = ADC_SSFIFO0_R; // read sample from the ADC sequence0 FIFO
		g_iADCBufferIndex = buffer_index;
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

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


//1eeogyeuV
