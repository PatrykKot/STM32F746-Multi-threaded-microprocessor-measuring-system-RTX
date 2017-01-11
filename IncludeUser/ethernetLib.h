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
#include "cmsis_os.h"

/**
 * @def ETHERNET_AMP_BUFFER_SIZE
 * @brief Amplitude data length sent to PC
 */
#define ETHERNET_AMP_BUFFER_SIZE 256

/**
 * HTTP request types
 */
 typedef enum {
	 NOT_SUPPORTED_REQUEST = 0,
	 GET_REQUEST,
	 PUT_REQUEST
 } HttpRequestType;
 
 typedef enum {
	 GET_REQUEST_SIGNAL = 0x01,
	 PUT_REQUEST_SIGNAL = 0x02,
	 DATA_SIGNAL = 0x03,
 } HttpServerSignal;

/**
 * UDP streaming port
 */
#define UDP_STREAMING_PORT 53426

/**
 * UDP streaming IP
 */
#define UDP_STREAMING_IP "192.168.1.10"

/**
 *
 */
#define UDP_STREAMING_FAILURE_TIMEOUT 3000

/* Functions */
void ethInit(void);
void printIp(void);
void printNetmask(void);
void printGateway(void);
uint32_t streamingSocketCallback(int32_t socket, const uint8_t *ip_addr, uint16_t port, const uint8_t *buf, uint32_t len);
void initStreamingSocket(void);
void openStreamingSocket(uint32_t port);
void getData(char* data);
netStatus sendSpectrum(SpectrumStr* spectrumStr, char* ipAddress, uint32_t port);
void copySpectrumToBuffer(uint8_t* buffer, float32_t* spectrumVector, uint32_t vectorLength);
void closeStreamingSocket(void);

uint32_t streamingSocketCallback(int32_t socket, const uint8_t *ip_addr, uint16_t port, const uint8_t *buf, uint32_t len);
uint32_t httpSocketCallback(int32_t soc, tcpEvent event, const uint8_t *buf, uint32_t len);
void initHttpSocket(osThreadId* thread_id);
void httpStartListen(void);
HttpRequestType getRequestType(char* fullMsg);
int32_t getHttpSocket(void);
void closeSocket(int32_t socket);
netStatus sendConfiguration(StmConfig* config, int32_t client, char* requestParameters);
netStatus sendHttpResponse(int32_t socket, char* httpStatus, char* requestParameters, char* content);
netStatus sendString(int32_t socket, const char* array);
uint8_t isConfigRequest(char* buf);
uint8_t isSystemRequest(char* buf);

#endif /* ETHERNETLIB_H_ */
