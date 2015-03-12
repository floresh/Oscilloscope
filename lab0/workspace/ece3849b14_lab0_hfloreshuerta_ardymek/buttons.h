/*
 * buttons.h
 *
 *  Created on: Aug 12, 2012
 *      Author: Gene Bogdanov
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

// Buttons functions

#define BUTTON_COUNT 5				// number of buttons
#define BUTTON_SAMPLES_RELEASED 5	// number of samples before a button is considered released
#define BUTTON_SAMPLES_PRESSED 2	// number of samples before a button is considered pressed
// counter value indicating button pressed state
#define BUTTON_PRESSED_STATE (BUTTON_SAMPLES_RELEASED*BUTTON_SAMPLES_PRESSED)

extern volatile unsigned long g_ulButtons;	// debounced button state, one per bit in the lowest bits

// update the debounced button state in the global variable g_ulButtons
// the input argument is a bitmap of raw button state from the hardware
void ButtonDebounce(unsigned long ulButtons);

#endif /* BUTTONS_H_ */
