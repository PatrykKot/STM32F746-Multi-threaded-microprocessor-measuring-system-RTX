/*
 * ethernetLib.h
 *
 *  Created on: 3 wrz 2016
 *      Author: Patryk Kotlarz
 */

#ifndef ETHERNETLIB_H_
#define ETHERNETLIB_H_

#include "stdint.h"
#include "lcdLogger.h"
#include "soundProcessing.h"
#include "jsonConfiguration.h"
#include "rl_net.h"
#include "rl_net_lib.h"

/**
 * @def ETHERNET_AMP_BUFFER_SIZE
 * @brief Amplitude data length sent to PC
 */
#define ETHERNET_AMP_BUFFER_SIZE 256

/**
 * HTTP request types
 */
#define NOT_SUPPORTED_REQUEST 0
#define GET_REQUEST 1
#define PUT_REQUEST 2

/**
 * UDP streaming port
 */
#define UDP_STREAMING_PORT 53426

/* Functions */
void ethInit(void);
void printIp(void);
void printNetmask(void);
void printGateway(void);
/*void printAddress(const struct netif* gnetif, uint8_t addressType);
uint32_t isEthernetCableConnected();
err_t sendSpectrum(SpectrumStr* ampStr, struct netconn *client);
uint8_t isNetconnStatusOk(err_t status);
err_t udpSend(struct netconn *client, void* buf, uint32_t buffSize);
void printContent(struct netbuf* buf);
uint16_t getRequestType(struct netbuf* buf);
err_t sendConfiguration(StmConfig* config, struct netconn* client,
		char* requestParameters);
err_t sendHttpResponse(struct netconn* client, char* httpStatus,
		char* requestParameters, char* content);
err_t sendString(struct netconn* client, const char* array);
uint8_t isConfigRequest(struct netbuf* buf);
uint8_t isSystemRequest(struct netbuf* buf);*/

#endif /* ETHERNETLIB_H_ */
