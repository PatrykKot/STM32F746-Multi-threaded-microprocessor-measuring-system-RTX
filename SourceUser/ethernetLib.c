/*
 * ethernetLib.c
 *
 *  Created on: 3 wrz 2016
 *      Author: Patryk Kotlarz
 */

#include "ethernetLib.h"

extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

static uint32_t streamingSocketHandle;

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

void eth_link_notify(uint32_t if_num, ethLinkEvent event)
{
	if(event != ethLinkDown)
	{
		logMsg("Ethernet link up");
	}
	else
	{
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

void openStreamingSocket(uint32_t port) {
	udp_open(streamingSocketHandle, port);
}

netStatus sendSpectrum(SpectrumStr* spectrumStr, char* ipAddress, uint32_t port) {
	//uint8_t ip;
	const uint8_t ip[4] = {192,168,1,10};
	uint8_t* buff;
	uint32_t length;
	
	length = ETHERNET_AMP_BUFFER_SIZE * sizeof(float32_t);	
	buff = udp_get_buf(length);
	//ip4_aton(ipAddress, &ip);
	
	copySpectrumToBuffer(buff, spectrumStr->amplitudeVector, ETHERNET_AMP_BUFFER_SIZE);
	
	return udp_send(streamingSocketHandle, ip, port, buff, length);
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

/**
 * @brief Used for printing the IP, netmask or gateway address
 * @param gnetif: pointer to \ref netif structure
 * @param addressType: type of address (IP_ADDRESS etc.)
 */
/*void printAddress(const struct netif* gnetif, uint8_t addressType) {
	char addrType[8];
	char msg[40];
	uint32_t address = 0;
	switch (addressType) {
	case IP_ADDRESS: {
		strcpy(addrType, "IP");
		address = gnetif->ip_addr.addr;
		break;
	}
	case NETMASK_ADDRESS: {
		strcpy(addrType, "Netmask");
		address = gnetif->netmask.addr;
		break;
	}
	case GATEWAY_ADDRESS: {
		strcpy(addrType, "Gateway");
		address = gnetif->gw.addr;
		break;
	}
	default:
		break;
	}
	sprintf(msg, "%s address: %d.%d.%d.%d", addrType, (uint8_t) (address),
			(uint8_t) (address >> 8), (uint8_t) (address >> 16),
			(uint8_t) (address >> 24));
	logMsg(msg);
}*/

/**
 * @brief The function checks state of ethernet cable (is it connected or not).
 * @retval \ref ETHERNET_CABLE_NOT_CONNECTED or \ref ETHERNET_CABLE_CONNECTED
 */
/*uint32_t isEthernetCableConnected() {
	uint32_t val;

	// reads from PHY register
	HAL_ETH_ReadPHYRegister(&EthHandle, 1, &val);
	val = val & (1 << 2);
	return val;
}*/

/**
 * @brief The function sends the \p ampStr by UDP to \p client.
 * @param ampStr: pointer to \ref AmplitudeStr
 * @param client: pointer to \ref netconn
 * @retval returns \ref ERR_OK if there are no errors
 */
/*err_t sendSpectrum(SpectrumStr* ampStr, struct netconn *client) {
	err_t status;

	if (client != NULL)
		if (client->state != NETCONN_CLOSE) {
			status = udpSend(client, ampStr->amplitudeVector,
			ETHERNET_AMP_BUFFER_SIZE * sizeof(float32_t));
			if (!isNetconnStatusOk(status))
				return status;
		}
	return ERR_OK;
}*/

/**
 * @brief The functions checks the returned \ref err_t because sometimes LWIP functions returns \ref ERR_RST if the ethernet cable is disconnected.
 * @param status: error code
 * @retval returns 0 if there are no errors
 */
/*uint8_t isNetconnStatusOk(err_t status) {
	if (status != ERR_OK && status != ERR_RST)
		return 0;
	return 1;
}*/

/**
 * @brief Used to send a some buffer \p buf to \p client by UDP
 * @param client: pointer to \ref netconn
 * @param buf: pointer to the beginning of data
 * @param buffSize: data length
 * @retval returns \ref ERR_OK if there are no errors
 */
/*err_t udpSend(struct netconn *client, void* buf, uint32_t buffSize) {
	err_t err;
	struct netbuf* netBuf = netbuf_new();
	err = netbuf_ref(netBuf, buf, buffSize);
	if (err != ERR_OK) {
		netbuf_delete(netBuf);
		return err;
	}
	err = netconn_send(client, netBuf);
	netbuf_delete(netBuf);
	return err;
}*/

/*void printContent(struct netbuf* buf) {
	void* data;
	uint16_t length;
	netbuf_data(buf, &data, &length);
	char* tok;
	char* fullMsg = (char*) data;
	tok = strtok((char*) fullMsg, "\n");
	while (tok != NULL) {
		logMsg(tok);
		tok = strtok(NULL, "\n");
	}
}*/

/**
 * @brief Returns the request type
 * @param buf: pointer to \ref netbuf structure
 * @retval GET_REQUEST, PUT_REQUEST or NOT_SUPPORTED_REQUEST
 */
/*uint16_t getRequestType(struct netbuf* buf) {
	void* data;
	uint16_t length;
	netbuf_data(buf, &data, &length);
	char* fullMsg = (char*) data;

	if (strstr(fullMsg, "GET") != NULL)
		return GET_REQUEST;
	else if (strstr(fullMsg, "PUT") != NULL)
		return PUT_REQUEST;
	else
		return NOT_SUPPORTED_REQUEST;
}*/

/*void printData(struct netbuf* buf) {
	void* data;
	uint16_t length;
	netbuf_data(buf, &data, &length);
	char* fullMsg = (char*) data;

	uint32_t len = strlen(fullMsg);
	uint32_t charInLine = 0;
	uint32_t lastLineEnding = 0;

	for (uint32_t i = 0; i < len; i++) {
		if ((fullMsg[i] == '\r' && fullMsg[i + 1] == '\n') || (i == len - 1)
				|| (charInLine > 40)) {
			char tempMsg[256];

			for (uint32_t j = lastLineEnding; j < i; j++) {
				tempMsg[j - lastLineEnding] = fullMsg[j];
			}
			tempMsg[i - lastLineEnding] = '\0';

			logMsg(tempMsg);
			lastLineEnding = i + 2;
			charInLine = 0;
		} else {
			charInLine++;
		}
	}
}*/

/**
 * @brief Sens the device configuration to the client
 * @param config: pointer to \ref StmConfig structure
 * @param client: pointer to \ref netconn structure (represents endpoint client)
 * @retval ERR_OK if there are no errors
 */
/*err_t sendConfiguration(StmConfig* config, struct netconn* client,
		char* requestParameters) {
	char configContent[256];
	stmConfigToString(config, configContent);
	return sendHttpResponse(client, "200 OK", requestParameters, configContent);
}*/

/*err_t sendHttpResponse(struct netconn* client, char* httpStatus,
		char* requestParameters, char* content) {
	char response[1024];
	sprintf(response, httpHeaderPattern, httpStatus, strlen(content),
			requestParameters, content);
	return sendString(client, response);
}

err_t sendString(struct netconn* client, const char* array) {
	return netconn_write(client, array, strlen(array), NETCONN_NOCOPY);
}*/

/*uint8_t contains(struct netbuf* buf, char* str) {
	void* data;
	uint16_t length;
	netbuf_data(buf, &data, &length);
	char* fullMsg = (char*) data;

	if (strstr(fullMsg, str) != NULL)
		return 1;
	return 0;
}*/

/**
 * @brief Check if the request includes '/config' text
 * @param buf: pointer to \ref netbuf structure
 * @retval 1 if request includes '/config'
 */
/*uint8_t isConfigRequest(struct netbuf* buf) {
	return contains(buf, " /config ");
}*/

/*uint8_t isSystemRequest(struct netbuf* buf) {
	return contains(buf, " /system ");
}*/
