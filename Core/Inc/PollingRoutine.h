/*
 * PollingRoutine.h
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template
 */

#ifndef INC_POLLINGROUTINE_H_
#define INC_POLLINGROUTINE_H_


/*

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#define Nop() asm(" NOP ")

#include "PollingRoutine.h"

*/
#ifndef __weak
#define __weak __attribute__((weak))
#endif

// Always measure VREFP voltage to calculate the 12 bit resolution.
// Assuming that VREF+ is 3.3V may result in incorrect calculations.
#define ADC_RESOLUTION 3.259 / 4096

typedef struct
{
	uint32_t adcValue;
	bool adcRdy;
}ADC_data_t;

void PollingInit(void);
void PollingRoutine(void);

void ADC_StartVbattConversion(void);
void ADC_CheckRdy(void);

#endif /* INC_POLLINGROUTINE_H_ */
