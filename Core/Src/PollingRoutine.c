/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern TimerCallbackStruct timerCallback;

UART_DMA_QueueStruct uart2_msg =
{
	.huart = &huart2,
	.rx.queueSize = UART_DMA_QUEUE_SIZE,
	.tx.queueSize = UART_DMA_QUEUE_SIZE
};

ADC_data_t adc_data = {0};

void PollingInit(void)
{
	// Calibrate ADC
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

	// Enable UART rx interrupt. Currently not receiving any messages.
	UART_DMA_EnableRxInterrupt(&uart2_msg);

	// Notify the user the STM32 is running
	UART_DMA_NotifyUser(&uart2_msg, "STM32 Ready", strlen("STM32 Ready"), true);

	// Create timercallback to start ADC conversion every 1 second
	TimerCallbackRegisterOnly(&timerCallback, ADC_StartVbattConversion);
	TimerCallbackTimerStart(&timerCallback, ADC_StartVbattConversion, 1000, TIMER_REPEAT);
}

void PollingRoutine(void)
{
	// Check if any functions need to be called.
	TimerCallbackCheck(&timerCallback);

	// Poll to see if ADC data is ready to be parsed.
	ADC_CheckRdy();
}

/*
 * Description: We will poll this and see if there was a ADC conversion completion.
 * 				From the data sheet, it indicates there is a bridge (voltage divider) with a reduction of 3.
 * 				We will take our ADC value and multiply by 3, then by the ADC resolution (3.259/4096).
 * 				We send this reading to a serial console.
 */
void ADC_CheckRdy(void)
{
	char str[UART_DMA_DATA_SIZE] = {0};
	float adcVoltage;

	if(adc_data.adcRdy)
	{
		adc_data.adcRdy = false;
		adcVoltage = adc_data.adcValue * 3 * ADC_RESOLUTION;
		sprintf(str, "vbatt = %f, ADC_Value = 0x%lX", adcVoltage, adc_data.adcValue);
		UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);
	}
}

/*
 * Description: This is called every 1 second to start the ADC conversion with interrupt
 */
void ADC_StartVbattConversion(void)
{
	if(HAL_ADC_Start_IT(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}
}

/*
 * Description: When the ADC is done with conversion, this is called.
 * 				We save the ADC results, set a flag and exit the interrupt.
 * 				Note: Never stay in the interrupt longer than needed. Always do calculations outside of interrupt.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc == &hadc1)
	{
		adc_data.adcValue = HAL_ADC_GetValue(hadc);
		adc_data.adcRdy = true;
	}
}

/*
 * Description: When the DMA is done transmitting, this is called.
 * 				We will call UART_DMA_SendMessage and transmit any pending messages in the queue
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == uart2_msg.huart)
	{
		uart2_msg.tx.txPending = false;
		UART_DMA_SendMessage(&uart2_msg);
	}
}
