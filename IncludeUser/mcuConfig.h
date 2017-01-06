/*
 * mcuConfig.c
 *
 *  Created on: Jul 5, 2016
 *      Author: Patryk Kotlarz
 */

#ifndef MCUCONFIG_H_
#define MCUCONFIG_H_

#include "stm32f7xx_hal.h"
#include "stm32f746xx.h"

void SystemClock_Config(void);
void Error_Handler(void);
#endif /* MCUCONFIG_H_ */
