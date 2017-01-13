/*
 * jsonConfiguration.c
 *
 *  Created on: 5 paz 2016
 *      Author: Patryk Kotlarz
 */

#include "jsonConfiguration.h"

static const char jsonTemplate[] = "{\"UdpEndpointPort\":%d,\"AmplitudeSamplingDelay\":%d,\"SamplingFrequency\":%d,\"UdpEndpointIP\":\"%s\",\"UdpDataSize\":%d}";

/**
 * @brief Parses JSON data to \StmConfig structure
 */
void parseJSON(char* jsonData, StmConfig* config) {
	cJSON* parser = cJSON_Parse(jsonData);
	
	config->amplitudeSamplingDelay = 0;
	config->audioSamplingFrequency = 0;
	strcpy(config->clientIp, "");
	config->clientPort = 0;
	config->ethernetDataSize = 0;
	
	if(cJSON_HasObjectItem(parser,"UdpEndpointIP"))
	{
		strcpy(config->clientIp, cJSON_GetObjectItem(parser, "UdpEndpointIP")->valuestring);
	}
	
	if(cJSON_HasObjectItem(parser,"AmplitudeSamplingDelay"))
	{
		config->amplitudeSamplingDelay = cJSON_GetObjectItem(parser, "AmplitudeSamplingDelay")->valueint;
	}
	
	if(cJSON_HasObjectItem(parser,"SamplingFrequency"))
	{
		config->audioSamplingFrequency = cJSON_GetObjectItem(parser, "SamplingFrequency")->valueint;
	}
	
	if(cJSON_HasObjectItem(parser,"UdpEndpointPort"))
	{
		config->clientPort = cJSON_GetObjectItem(parser, "UdpEndpointPort")->valueint;
	}
	
	if(cJSON_HasObjectItem(parser,"UdpDataSize"))
	{
		config->ethernetDataSize = cJSON_GetObjectItem(parser, "UdpDataSize")->valueint;
	}

	cJSON_Delete(parser);
}

/**
 * @brief Converts \ref StmConfig structure to JSON string
 * @param config: pointer to \ref StmConfig structure
 * @param str: pointer to output of the JSON string (must have allocated memory)
 */
void stmConfigToString(StmConfig* config, char* str) {
	sprintf(str, jsonTemplate, config->clientPort, config->amplitudeSamplingDelay, config->audioSamplingFrequency, config->clientIp, config->ethernetDataSize);
}

/**
 * @brief Copies \ref StmConfig structure to another \ref StmConfig structure
 * @param destination: pointer (output) to \ref StmConfig structure
 * @param source: pointer to \ref StmConfig structure
 */
void copyConfig(StmConfig* destination, StmConfig* source) {
	if(source->amplitudeSamplingDelay != 0)
	{
		destination->amplitudeSamplingDelay = source->amplitudeSamplingDelay;
	}
	
	if(source->audioSamplingFrequency != 0)
	{
		destination->audioSamplingFrequency = source->audioSamplingFrequency;
	}
	
	if(strlen(source->clientIp)>0)
	{
		strcpy(destination->clientIp, source->clientIp);
	}
	
	if(source->clientPort != 0)
	{
		destination->clientPort = source->clientPort;
	}
	
	if(source->ethernetDataSize != 0)
	{
		destination->ethernetDataSize = source->ethernetDataSize;
	}
		
}

void makeChanges(StmConfig* newConfig, StmConfig* oldConfig) {
	char msg[30];
	
	if(newConfig->amplitudeSamplingDelay != oldConfig->amplitudeSamplingDelay && newConfig->amplitudeSamplingDelay != 0)
	{
		logMsgVal("Changed sampling delay ", newConfig->amplitudeSamplingDelay);
	}
	
	if(newConfig->audioSamplingFrequency != oldConfig->audioSamplingFrequency && newConfig->audioSamplingFrequency != 0)
	{
		audioRecorderSetSamplingFrequency(newConfig->audioSamplingFrequency);
		logMsgVal("Changed sampling frequency ", newConfig->audioSamplingFrequency);
	}
	
	if(newConfig->clientPort != oldConfig->clientPort && newConfig->clientPort != 0)
	{
		logMsgVal("Changed client port ", newConfig->clientPort);
	}
	
	if(strcmp(newConfig->clientIp, oldConfig->clientIp) && strlen(newConfig->clientIp) > 0)
	{
		sprintf(msg, "Changed client IP: %s", newConfig->clientIp);
		logMsg(msg);
	}
	
	if(newConfig->ethernetDataSize != oldConfig->ethernetDataSize && newConfig->ethernetDataSize != 0)
	{
		logMsgVal("Changed UDP data size ", newConfig->ethernetDataSize);
	}
}
