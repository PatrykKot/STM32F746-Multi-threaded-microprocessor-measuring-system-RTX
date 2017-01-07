/*
 * main.c
 *
 *  Created on: 1 wrz 2016
 *      Author: Patryk Kotlarz
 */


#include "main.h"

/**
 * Main function
 */
int main(void) {
	/* CORE INITIALIZATION */

	/* HAL initialization */
	HAL_Init();

	/* Setting the system clock */
	SystemClock_Config();

	osKernelInitialize();
	threadsInit();
	
	osKernelStart();

	while (1)
	{
		osThreadYield ();
	}
}
