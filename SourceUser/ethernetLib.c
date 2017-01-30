/*
 * ethernetLib.c
 *
 *  Created on: 3 wrz 2016
 *      Author: Patryk Kotlarz
 */

#include "ethernetLib.h"

extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

/**
 * @brief Ethernet cable state
 */
uint8_t ethernetConnected = 0;

/**
 * @brief Handle to UDP socket
 **/
static uint32_t streamingSocketHandle;

/**
 * @brief Handle to TCP socket
 */
static uint32_t httpSocketHandle;

/**
 * @brief Temporary pointer to HTTP data
 */
static char httpData[1024];

/**
 * @brief HTTP thread handle
 */
static osThreadId* httpThreadHandle;

/**
 * @brief Union to easy convert integer value to byte float value (and inverse)
 */
union FloatConversion {
	float32_t floatVal;
	uint32_t intVal;
};

/**
 * @var char httpOkHeaderPattern[]
 * @brief Plain header of HTTP response
 */
const char httpHeaderPattern[] = "HTTP/1.0 %s\r\nContent-Length: %d%s\r\n\r\n%s";

/**
 * @brief Ethernet initialization
 */
void ethInit(void) {
	net_initialize();
}

/**
 * @brief Checks if is ethernet cable connected
 * @retval 1 if is connected
 */
uint8_t isEthernetConnected()
{
	return ethernetConnected;
}

/**
 * @brief Ethernet interface state interrupt
 * @param event: ethernet state type
 */
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

/**
 * @brief Prints ethernet interface IP on LCD
 */
void printIp() {
	char msg[30];
	sprintf(msg, "IP: %s", ip4_ntoa (LocM.IpAddr));
	logMsg(msg);
}

/**
 * @brief Prints ethernet interface netmask on LCD
 */
void printNetmask() {
	char msg[30];
	sprintf(msg, "Netmask: %s", ip4_ntoa (LocM.NetMask));
	logMsg(msg);
}

/**
 * @brief Prints ethernet interface gateway on LCD
 */
void printGateway() {
	char msg[30];
	sprintf(msg, "Gateway: %s", ip4_ntoa (LocM.DefGW));
	logMsg(msg);
}

/**
 * @brief UDP socket interrupt
 */
uint32_t streamingSocketCallback(int32_t socket, const uint8_t *ip_addr, uint16_t port, const uint8_t *buf, uint32_t len) {
	return 0;
}

/**
 * @brief Initializes UDP socket
 */
void initStreamingSocket() {
	streamingSocketHandle = udp_get_socket (0, 0, streamingSocketCallback);
}

/**
 * @brief Opens UDP socket
 * @param config: pointer to system configuration structure
 */
void openStreamingSocket(StmConfig* config) {
	udp_open(streamingSocketHandle, config->clientPort);
}

/**
 * @brief Sends spectrum by UDP
 * @param spectrumStr: pointer to spectrum buffer
 * @param config: pointer to system configuration structure
 * @retval network status
 */
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

/**
 * @brief Copies spectrum buffer to integer array
 * @param buffer: output integer array
 * @param spectrumVector: spectrum buffer
 * @param vectorLength: spectrum buffer length
 */
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

/**
 * @brief Closes UDP socket
 */
void closeStreamingSocket() {
	udp_close(streamingSocketHandle);
}

/**
 * @brief HTTP socket interrupt
 * @param soc: socket handle
 * @param event: event type
 * @param buf: incoming data
 * @param buf: data length
 * @retval returns 1 if TCP server should accept new connection
 */
uint32_t httpSocketCallback(int32_t soc, tcpEvent event, const uint8_t *buf, uint32_t len) {
	uint16_t requestType;
	char* request;
	
	request = (char*)buf;
  switch (event) {
    case tcpEventConnect:
      return (1);
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

/**
 * @brief Initializes TCP socket
 * @param thread_id: pointer to HTTP system thread
 */
void initHttpSocket(osThreadId* thread_id) {
	httpSocketHandle = tcp_get_socket(TCP_TYPE_SERVER, 0, 50, httpSocketCallback);
	httpThreadHandle = thread_id;
}

/**
 * @brief Last data buffer from TCP socket
 * @param data: output data pointer
 */
void getData(char* data) {
	strcpy(data, httpData);
}

/**
 * @brief Starts HTTP server listening
 */
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

/**
 * @brief Gets TCP socket handle
 * @retval HTTP socket handle
 */
int32_t getHttpSocket() {
	return httpSocketHandle;
}

/**
 * @brief Closes TCP socket
 * @param socket: socket handle
 */
void closeSocket(int32_t socket)
{
	tcp_close(socket);
}

/**
 * @brief Sends the device configuration to the client
 * @param config: pointer to \ref StmConfig structure
 * @param client: client handle
 * @param requestParameters: HTTP request parameters
 * @retval ERR_OK if there are no errors
 */
netStatus sendConfiguration(StmConfig* config, int32_t client, char* requestParameters) {
	char configContent[256];
	stmConfigToString(config, configContent, 256);
	return sendHttpResponse(client, "200 OK", requestParameters, configContent);
}

/**
 * @brief Sends HTTP response to the client
 * @param client: client handle
 * @param httpStatus: HTTP status
 * @param requestParameters: HTTP request parameters
 * @param content: HTTP content
 * @retval ERR_OK if there are no errors
 */
netStatus sendHttpResponse(int32_t client, char* httpStatus, char* requestParameters, char* content) {
	char response[1024];
	sprintf(response, httpHeaderPattern, httpStatus, strlen(content),
			requestParameters, content);
	return sendString(client, response);
}

/**
 * @brief Sends string by TCP
 * @param array: string to send
 * @retval ERR_OK if there are no errors
 */
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
		logErr("Check TCP false");
		return netBusy;
	}
}

/**
 * @brief Check if the request includes '/config' text
 * @param buf: pointer to \ref netbuf structure
 * @retval returns 1 if request includes '/config'
 */
uint8_t isConfigRequest(char* buf) {
	return (strstr(buf, " /config ")!=NULL);
}

/**
 * @brief Check if the request includes '/system' text
 * @param buf: pointer to \ref netbuf structure
 * @retval returns 1 if request includes '/system'
 */
uint8_t isSystemRequest(char* buf) {
	return (strstr(buf, " /system ")!=NULL);
}
