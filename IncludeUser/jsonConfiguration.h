/*
 * jsonConfiguration.h
 *
 *  Created on: 5 paü 2016
 *      Author: Patryk Kotlarz
 */

#ifndef JSONCONFIGURATION_H_
#define JSONCONFIGURATION_H_

#define ARM_MATH_CM7

#include "stm32f746xx.h"
#include "stdint.h"
#include "arm_math.h"
#include "lcdLogger.h"
#include "cJSON.h"
#include "audioRecording.h"
#include "soundProcessing.h"

#define TRUE 1
#define FALSE 0

/**
 * @brief Structure which represents device configuration
 */
typedef struct {
	uint8_t amplitudeSamplingDelay;
	uint32_t audioSamplingFrequency;
	uint32_t clientPort;
	uint32_t windowType;
	char clientIp[20];
} StmConfig;

/* Functions */
void parseJSON(char* jsonData, StmConfig* config);
void stmConfigToString(StmConfig* config, char* str, uint32_t len);
void makeChanges(StmConfig* newConfig, StmConfig* oldConfig);

#endif /* JSONCONFIGURATION_H_ */

