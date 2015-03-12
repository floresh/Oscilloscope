/*
 * network.h
 *
 *  Created on: Nov 23, 2012
 *      Author: Gene Bogdanov
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#define CAN_BIT_RATE 1000000 // [bps] CAN bit rate, 1 Mbps max

#define MSG_ID_RX 2 // message IDs 11-bit or 29-bit
#define MSG_ID_TX 1 // one of the communicating boards should have these swapped

#define MSG_NUM_RX 1 // message number 1...32
#define MSG_NUM_TX 2 // these can be constant on both communicating boards

// CAN interrupt service routine - must be registered with the interrupt controller/dispatcher
void CAN_ISR(void);

// initialize CAN
void NetworkInit(void);

// transmit 32-bit data on CAN using the MSG_ID_TX message ID
void NetworkTx(unsigned long ulData);

// called by the CAN ISR when a message is received with MSG_ID_RX message ID
// the argument is the received 32-bit data
void NetworkRxCallback(unsigned long ulData);

#endif /* NETWORK_H_ */
