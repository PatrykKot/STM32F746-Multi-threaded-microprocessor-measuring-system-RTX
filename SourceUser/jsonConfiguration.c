/*
 * jsonConfiguration.c
 *
 *  Created on: 5 pa� 2016
 *      Author: Patryk Kotlarz
 */

#include "jsonConfiguration.h"

/**
 * @brief Parses \netbuf (JSON data) to \StmConfig structure
 */
/*void parseJSON(struct netbuf* buf, StmConfig* config) {
	void* data;
	uint16_t length;
	netbuf_data(buf, &data, &length);
	char* jsonData = (char*) data;

	cJSON* parser = cJSON_Parse(jsonData);*/
	/*config->started = cJSON_GetObjectItem(parser, "Started")->valueint;*/
	/*char* udpEndpoint =
			cJSON_GetObjectItem(parser, "UdpEndpointIP")->valuestring;*/
	/*config->udpEndpointPort =
	 cJSON_GetObjectItem(parser, "UdpEndpointPort")->valueint;*/
	/*config->amplitudeSamplingDelay = cJSON_GetObjectItem(parser,
			"AmplitudeSamplingDelay")->valueint;
	config->audioSamplingFrequency = cJSON_GetObjectItem(parser,
			"SamplingFrequency")->valueint;
	config->clientPort =
			cJSON_GetObjectItem(parser, "UdpEndpointPort")->valueint;

	int ipTab[4];
	sscanf(udpEndpoint, "%d.%d.%d.%d", &ipTab[0], &ipTab[1], &ipTab[2],
			&ipTab[3]);
	IP4_ADDR(&config->clientIp, ipTab[0], ipTab[1], ipTab[2], ipTab[3]);

	cJSON_Delete(parser);
}*/

/**
 * @brief Converts \ref StmConfig structure to JSON string
 * @param config: pointer to \ref StmConfig structure
 * @param str: pointer to output of the JSON string (must have allocated memory)
 */
void stmConfigToString(StmConfig* config, char* str) {
	cJSON *jsonCreator;
	char* json;
		
	jsonCreator = cJSON_CreateObject();
	/*cJSON_AddBoolToObject(jsonCreator, "Started", config->started);*/
	cJSON_AddNumberToObject(jsonCreator, "UdpEndpointPort", config->clientPort);
	cJSON_AddNumberToObject(jsonCreator, "AmplitudeSamplingDelay",
			config->amplitudeSamplingDelay);
	cJSON_AddNumberToObject(jsonCreator, "SamplingFrequency",
			config->audioSamplingFrequency);

	/*char ip[15];
	int ipTab[4];
	for (uint8_t i = 0; i < 4; i++) {
		ipTab[i] = IP_ADDR_GET(config->clientIp, i);
	}
	sprintf(ip, "%d.%d.%d.%d", ipTab[0], ipTab[1], ipTab[2], ipTab[3]);
	cJSON_AddStringToObject(jsonCreator, "UdpEndpointIP", ip);*/

	json = cJSON_Print(jsonCreator);
	strcpy(str, json);
	cJSON_Delete(jsonCreator);
	free(json);
}

/**
 * @brief Copies \ref StmConfig structure to another \ref StmConfig structure
 * @param destination: pointer (output) to \ref StmConfig structure
 * @param source: pointer to \ref StmConfig structure
 */
void copyConfig(StmConfig* destination, StmConfig* source) {
	destination->amplitudeSamplingDelay = source->amplitudeSamplingDelay;
	destination->audioSamplingFrequency = source->audioSamplingFrequency;
	//destination->clientIp.addr = source->clientIp.addr;
	destination->clientPort = source->clientPort;

	/*destination->started = source->started;
	 for (uint16_t i = 0; i < 4; i++)
	 destination->udpEndpointAddr[i] = source->udpEndpointAddr[i];
	 destination->udpEndpointPort = source->udpEndpointPort;*/
}