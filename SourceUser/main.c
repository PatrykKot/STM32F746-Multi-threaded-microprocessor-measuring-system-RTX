/*
 * main.c
 *
 *  Created on: 1 wrz 2016
 *      Author: Patryk Kotlarz
 */


#include "main.h"

#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

void simpleTask(void const* argument)
{
	while(1)
	{
		osDelay(1000);
		logMsg("Task");
	}
}

/**
 * Main function
 */
int main(void) {
	/* CORE INITIALIZATION */

	/* HAL initialization */
	HAL_Init();

	/* Setting the system clock */
	SystemClock_Config();

	threadsInit();
	osKernelStart();

	while (1)
		;
}
