/*
 * jsonConfiguration.h
 *
 *  Created on: 5 pa� 2016
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

#define IP_ADDR_GET(ipaddr,index) (int)(((u32_t)(ipaddr.addr)>>((u32_t)(8*index)))&((u32_t)0xff))

#define TRUE 1
#define FALSE 0

/**
 * @brief Structure represents device configuration
 */
typedef struct {
	uint8_t amplitudeSamplingDelay;
	uint32_t audioSamplingFrequency;
	uint32_t clientPort;
	uint32_t ethernetDataSize;
	//uint32_t audioVolume;
	//uint8_t systemStarted;
	char clientIp[20];
} StmConfig;

/* Functions */
void parseJSON(char* jsonData, StmConfig* config);
void stmConfigToString(StmConfig* config, char* str);
void copyConfig(StmConfig* destination, StmConfig* source);
void makeChanges(StmConfig* newConfig, StmConfig* oldConfig);

#endif /* JSONCONFIGURATION_H_ */

