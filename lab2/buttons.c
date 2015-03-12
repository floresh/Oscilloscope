/*
 * buttons.c
 *
 *  Created on: Aug 12, 2012
 *      Author: Gene Bogdanov
 */
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/lm3s8962.h"
#include "buttons.h"

// public global
volatile unsigned long g_ulButtons;	// debounced button state, one per bit in the lowest bits

// update the debounced button state g_ulButtons
void ButtonDebounce(unsigned long ulButtons)
{
	int i, mask;
	static int piState[BUTTON_COUNT]; // button state: 0 = released
									  // BUTTON_PRESSED_STATE = pressed
									  // in between = previous state
	for (i = 0; i < BUTTON_COUNT; i++) {
		mask = 1 << i;
		if (ulButtons & mask) {
			piState[i] += BUTTON_PRESSED_STATE/BUTTON_SAMPLES_PRESSED;
			if (piState[i] >= BUTTON_PRESSED_STATE) {
				piState[i] = BUTTON_PRESSED_STATE;
				g_ulButtons |= mask; // update button status
			}
		}
		else {
			piState[i] -= BUTTON_PRESSED_STATE/BUTTON_SAMPLES_RELEASED;
			if (piState[i] <= 0) {
				piState[i] = 0;
				g_ulButtons &= ~mask;
			}
		}
	}
}
