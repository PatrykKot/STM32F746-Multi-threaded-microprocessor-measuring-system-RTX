/*
 * jsonConfiguration.c
 *
 *  Created on: 5 paz 2016
 *      Author: Patryk Kotlarz
 */

#include "jsonConfiguration.h"

static const char jsonTemplate[] = "{\"UdpEndpointPort\":%d,\"AmplitudeSamplingDelay\":%d,\"SamplingFrequency\":%d,\"UdpEndpointIP\":\"%s\",\"SystemStarted\":%s,\"AudioVolume\":%d,\"UdpDataSize\":%d}";

/**
 * @brief Parses \netbuf (JSON data) to \StmConfig structure
 */
void parseJSON(char* jsonData, StmConfig* config) {
	cJSON* parser = cJSON_Parse(jsonData);
	
	strcpy(config->clientIp, cJSON_GetObjectItem(parser, "UdpEndpointIP")->valuestring);
	config->amplitudeSamplingDelay = cJSON_GetObjectItem(parser, "AmplitudeSamplingDelay")->valueint;
	config->audioSamplingFrequency = cJSON_GetObjectItem(parser, "SamplingFrequency")->valueint;
	config->clientPort = cJSON_GetObjectItem(parser, "UdpEndpointPort")->valueint;
	//config->systemStarted = cJSON_GetObjectItem(parser, "SystemStarted")->valueint;
	//config->audioVolume = cJSON_GetObjectItem(parser, "AudioVolume")->valueint;
	config->ethernetDataSize = cJSON_GetObjectItem(parser, "UdpDataSize")->valueint;

	cJSON_Delete(parser);
}

/**
 * @brief Converts \ref StmConfig structure to JSON string
 * @param config: pointer to \ref StmConfig structure
 * @param str: pointer to output of the JSON string (must have allocated memory)
 */
void stmConfigToString(StmConfig* config, char* str) {
	char systemStarted[6];
	
	/*if(config->systemStarted)
	{
		strcpy(systemStarted, "true");
	}
	else
	{
		strcpy(systemStarted, "false");
	}*/
	
	sprintf(str, jsonTemplate, config->clientPort, config->amplitudeSamplingDelay, config->audioSamplingFrequency, config->clientIp, "true", 10, config->ethernetDataSize);
}

/**
 * @brief Copies \ref StmConfig structure to another \ref StmConfig structure
 * @param destination: pointer (output) to \ref StmConfig structure
 * @param source: pointer to \ref StmConfig structure
 */
void copyConfig(StmConfig* destination, StmConfig* source) {
	destination->amplitudeSamplingDelay = source->amplitudeSamplingDelay;
	destination->audioSamplingFrequency = source->audioSamplingFrequency;
	strcpy(destination->clientIp, source->clientIp);
	destination->clientPort = source->clientPort;
	//destination->audioVolume = source->audioVolume;
	destination->ethernetDataSize = source->ethernetDataSize;
	//destination->systemStarted = source->systemStarted;
}

void makeChanges(StmConfig* newConfig, StmConfig* oldConfig) {
	char msg[30];
	
	if(newConfig->amplitudeSamplingDelay != oldConfig->amplitudeSamplingDelay)
	{
		logMsgVal("Changed sampling delay ", newConfig->amplitudeSamplingDelay);
	}
	
	if(newConfig->audioSamplingFrequency != oldConfig->audioSamplingFrequency)
	{
		audioRecorderSetSamplingFrequency(newConfig->audioSamplingFrequency);
		logMsgVal("Changed sampling frequency ", newConfig->audioSamplingFrequency);
	}
	
	if(newConfig->clientPort != oldConfig->clientPort)
	{
		logMsgVal("Changed client port ", newConfig->clientPort);
	}
	
	if(strcmp(newConfig->clientIp, oldConfig->clientIp))
	{
		sprintf(msg, "Changed client IP: %s", newConfig->clientIp);
		logMsg(msg);
	}
	
	if(newConfig->ethernetDataSize != oldConfig->ethernetDataSize)
	{
		logMsgVal("Changed UDP data size ", newConfig->ethernetDataSize);
	}
	
	/*if(newConfig->audioVolume != newConfig->audioVolume)
	{
		audioRecorderSetVolume(newConfig->audioVolume);
		logMsgVal("Changed audio volume ", newConfig->audioVolume);
	}
	
	if(newConfig->systemStarted != newConfig->systemStarted)
	{
		if(newConfig->systemStarted == TRUE)
		{
			logMsg("System started");
		}
		else
		{
			logMsg("System stopped");
		}
	}*/
}
