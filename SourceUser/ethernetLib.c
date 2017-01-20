/*
 * ethernetLib.c
 *
 *  Created on: 3 wrz 2016
 *      Author: Patryk Kotlarz
 */

#include "ethernetLib.h"

extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]
uint8_t ethernetConnected = 0;

static uint32_t streamingSocketHandle;
static uint32_t httpSocketHandle;

static char httpData[1024];
static osThreadId* httpThreadHandle;

union FloatConversion {
	float32_t floatVal;
	uint32_t intVal;
};

/**
 * @var char httpOkHeaderPattern[]
 * @brief Plain header of 200 HTTP response
 */
const char httpHeaderPattern[] = "HTTP/1.0 %s\r\nContent-Length: %d%s\r\n\r\n%s";

void ethInit(void) {
	net_initialize();
}

uint8_t isEthernetConnected()
{
	return ethernetConnected;
}

void eth_link_notify(uint32_t if_num, ethLinkEvent event)
{
	if(event != ethLinkDown)
	{
		ethernetConnected = 1;
		logMsg("Ethernet link up");
	}
	else
	{
		ethernetConnected = 0;
		logMsg("Ethernet link down");
	}
}

void printIp() {
	char msg[30];
	sprintf(msg, "IP: %s", ip4_ntoa (LocM.IpAddr));
	logMsg(msg);
}

void printNetmask() {
	char msg[30];
	sprintf(msg, "Netmask: %s", ip4_ntoa (LocM.NetMask));
	logMsg(msg);
}

void printGateway() {
	char msg[30];
	sprintf(msg, "Gateway: %s", ip4_ntoa (LocM.DefGW));
	logMsg(msg);
}

uint32_t streamingSocketCallback(int32_t socket, const uint8_t *ip_addr, uint16_t port, const uint8_t *buf, uint32_t len) {
	logMsg("Streaming socket callback");
	
  return 0;
}

void initStreamingSocket() {
	streamingSocketHandle = udp_get_socket (0, 0, streamingSocketCallback);
}

void openStreamingSocket(StmConfig* config) {
	udp_open(streamingSocketHandle, config->clientPort);
}

netStatus sendSpectrum(SpectrumStr* spectrumStr, StmConfig* config) {
	uint8_t ip[4];
	uint8_t* buff;
	uint32_t length;
	
	sscanf(config->clientIp, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	
	length = ETHERNET_DEFAULT_AMP_BUFFER_SIZE * sizeof(float32_t);	
	buff = udp_get_buf(length);
	
	copySpectrumToBuffer(buff, spectrumStr->amplitudeVector, ETHERNET_DEFAULT_AMP_BUFFER_SIZE);
	
	return udp_send(streamingSocketHandle, ip, config->clientPort, buff, length);
}

void copySpectrumToBuffer(uint8_t* buffer, float32_t* spectrumVector, uint32_t vectorLength) {
	uint32_t intIterator;
	uint32_t floatIterator;
	uint32_t floatSizeIterator;
	uint32_t floatSize;
	union FloatConversion conv;
	
	floatSize = sizeof(float32_t);
	intIterator = 0;
	
	for(floatIterator = 0; floatIterator < vectorLength; floatIterator++)
	{
		conv.floatVal = spectrumVector[floatIterator];
		for(floatSizeIterator = 0; floatSizeIterator < floatSize; floatSizeIterator++)
		{
			buffer[intIterator++] = (uint8_t)(conv.intVal >> ((0x08 * floatSizeIterator)));
		}
	}
}

void closeStreamingSocket() {
	udp_close(streamingSocketHandle);
}

uint32_t httpSocketCallback(int32_t soc, tcpEvent event, const uint8_t *buf, uint32_t len) {
	uint16_t requestType;
	char* request;
	
	request = (char*)buf;
  switch (event) {
    case tcpEventConnect:
			//logMsg("New connection");
      return (1);
    case tcpEventAbort:
      //logErr("Connection aborted");
      break;
    case tcpEventEstablished:
      //logMsg("Connected to peer"); 
      break;
    case tcpEventClosed:
      //logMsg("Connection closed");
      break;
    case tcpEventACK:
      //logMsg("Event ACK");
      break;
    case tcpEventData:
		{
      logMsg("Event data");
			requestType = getRequestType(request);
			switch(requestType)
			{
				case GET_REQUEST:
				{
					logMsg("GET");
					logMsg(request);
					strcpy(httpData, request);
					osSignalSet(*httpThreadHandle, GET_REQUEST_SIGNAL);
					break;
				}
				case PUT_REQUEST:
				{
					logMsg("PUT");
					logMsg(request);
					strcpy(httpData, request);
					osSignalSet(*httpThreadHandle, PUT_REQUEST_SIGNAL);
					break;
				}
				default:
				{
					logMsg("DATA");
					logMsg(request);
					strcpy(httpData, request);
					osSignalSet(*httpThreadHandle, HTTP_DATA_SIGNAL);
					break;
				}
			}
			break;
		}
  }
  return 0;
}

void initHttpSocket(osThreadId* thread_id) {
	httpSocketHandle = tcp_get_socket(TCP_TYPE_SERVER, 0, 50, httpSocketCallback);
	httpThreadHandle = thread_id;
}

void getData(char* data) {
	strcpy(data, httpData);
}

void httpStartListen() {
	tcp_listen(httpSocketHandle, 80);
}

/**
 * @brief Returns the request type
 * @param buf: pointer to \ref netbuf structure
 * @retval GET_REQUEST, PUT_REQUEST or NOT_SUPPORTED_REQUEST
 */
HttpRequestType getRequestType(char* fullMsg) {
	if (strstr(fullMsg, "GET") != NULL)
		return GET_REQUEST;
	else if (strstr(fullMsg, "PUT") != NULL)
		return PUT_REQUEST;
	else
		return NOT_SUPPORTED_REQUEST;
}

int32_t getHttpSocket() {
	return httpSocketHandle;
}

void closeSocket(int32_t socket)
{
	tcp_close(socket);
}

/**
 * @brief Sens the device configuration to the client
 * @param config: pointer to \ref StmConfig structure
 * @param client: pointer to \ref netconn structure (represents endpoint client)
 * @retval ERR_OK if there are no errors
 */
netStatus sendConfiguration(StmConfig* config, int32_t client, char* requestParameters) {
	char configContent[256];
	stmConfigToString(config, configContent, 256);
	return sendHttpResponse(client, "200 OK", requestParameters, configContent);
}

netStatus sendHttpResponse(int32_t client, char* httpStatus,
		char* requestParameters, char* content) {
	char response[1024];
	sprintf(response, httpHeaderPattern, httpStatus, strlen(content),
			requestParameters, content);
	return sendString(client, response);
}

netStatus sendString(int32_t client, const char* array) {
	uint32_t length;
	uint8_t* buf;
	
	if(tcp_check_send(client))
	{
		length = strlen(array);
		buf = tcp_get_buf(length);
		memcpy (buf, array, length);
		return tcp_send(client, buf, length);
	}
	else
	{
		logErr("Check false");
		return netBusy;
	}
}

/**
 * @brief Check if the request includes '/config' text
 * @param buf: pointer to \ref netbuf structure
 * @retval 1 if request includes '/config'
 */
uint8_t isConfigRequest(char* buf) {
	return (strstr(buf, " /config ")!=NULL);
}

uint8_t isSystemRequest(char* buf) {
	return (strstr(buf, " /system ")!=NULL);
}

uint8_t isWindowSizeRequest(char* buf) {
	return (strstr(buf, " /windowSize ")!=NULL);
}
