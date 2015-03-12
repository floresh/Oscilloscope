/*
 * network.c
 *
 *  Created on: Nov 23, 2012
 *      Author: Gene Bogdanov
 */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
//#include "inc/lm3s8962.h" // LM3S8962 specific
//#include "lm3s2110.h"     // LM3S2110 specific
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "network.h"

static tCANMsgObject g_MsgRx;
static unsigned long g_ulRxData;

static tCANMsgObject g_MsgTx;

void NetworkMsgInit(void);

void CAN_ISR(void)
{
    unsigned long ulStatus;

    ulStatus = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);
    switch(ulStatus)
    {
    case MSG_NUM_RX: // message received
    	CANMessageGet(CAN0_BASE, MSG_NUM_RX, &g_MsgRx, /*clear int*/ 1);
    	NetworkRxCallback(g_ulRxData); // call user function upon receiving a message
    	break;
    default: // status or other interrupt: clear it without further action
    	CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
    	CANIntClear(CAN0_BASE, ulStatus);
    }
}

void NetworkInit(void)
{
	// configure GPIO pins for CAN
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeCAN(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // set up the CAN controller
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
    CANInit(CAN0_BASE);
    CANBitRateSet(CAN0_BASE, 8000000, CAN_BIT_RATE);
    CANEnable(CAN0_BASE);
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
    NetworkMsgInit();

//    volatile int i;
//    CAN0_CTL_R;
//    for (i=0;i<8;i++);
//    CAN0_CTL_R |= CAN_CTL_TEST; // enable CAN test mode
//    for (i=0;i<8;i++);
//    CAN0_TST_R;
//    for (i=0;i<8;i++);
//    CAN0_TST_R |= CAN_TST_LBACK; // enable loopback mode

    IntPrioritySet(INT_CAN0, 96);
    IntEnable(INT_CAN0);
}

void NetworkMsgInit(void)
{
	g_MsgRx.ulMsgID = MSG_ID_RX;
	g_MsgRx.ulMsgIDMask = 0;
	g_MsgRx.ulFlags = MSG_OBJ_RX_INT_ENABLE;
	g_MsgRx.ulMsgLen = 4;
	g_MsgRx.pucMsgData = (unsigned char *)&g_ulRxData;
    CANMessageSet(CAN0_BASE, MSG_NUM_RX, &g_MsgRx, MSG_OBJ_TYPE_RX);

    g_MsgTx.ulMsgID = MSG_ID_TX;
    g_MsgTx.ulMsgIDMask = 0;
    g_MsgTx.ulFlags = 0;
    g_MsgTx.ulMsgLen = 4;
}

void NetworkTx(unsigned long ulData)
{
	g_MsgTx.pucMsgData = (unsigned char *)&ulData;
	CANMessageSet(CAN0_BASE, MSG_NUM_TX, &g_MsgTx, MSG_OBJ_TYPE_TX);
}

void NetworkRxCallback(unsigned long ulData)
{

}
