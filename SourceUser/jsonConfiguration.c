/*
 * jsonConfiguration.c
 *
 *  Created on: 5 paz 2016
 *      Author: Patryk Kotlarz
 */

#include "jsonConfiguration.h"

static const char jsonTemplate[] = "{\"UdpEndpointPort\":%d,\"AmplitudeSamplingDelay\":%d,\"SamplingFrequency\":%d,\"UdpEndpointIP\":\"%s\"}";

/**
 * @brief Parses \netbuf (JSON data) to \StmConfig structure
 */
void parseJSON(char* jsonData, StmConfig* config) {
	cJSON* parser = cJSON_Parse(jsonData);
	strcpy(config->clientIp, cJSON_GetObjectItem(parser, "UdpEndpointIP")->valuestring);
	config->amplitudeSamplingDelay = cJSON_GetObjectItem(parser,
			"AmplitudeSamplingDelay")->valueint;
	config->audioSamplingFrequency = cJSON_GetObjectItem(parser,
			"SamplingFrequency")->valueint;
	config->clientPort =
			cJSON_GetObjectItem(parser, "UdpEndpointPort")->valueint;

	cJSON_Delete(parser);
}

/**
 * @brief Converts \ref StmConfig structure to JSON string
 * @param config: pointer to \ref StmConfig structure
 * @param str: pointer to output of the JSON string (must have allocated memory)
 */
void stmConfigToString(StmConfig* config, char* str) {
	sprintf(str, jsonTemplate, config->clientPort, config->amplitudeSamplingDelay, config->audioSamplingFrequency, config->clientIp);
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
}
