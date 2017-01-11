/*
 * usrTasks.c
 *
 *  Created on: 14 wrz 2016
 *      Author: Patryk Kotlarz
 */

#include "usrTasks.h"

StmConfig configStr;

/**
 * @var SoundBuffer* mainSoundBuffer
 * @brief Cyclic buffer which holds audio samples
 */
SoundBufferStr* mainSoundBuffer;

/**
 * @var uint16_t dmaAudioBuffer[AUDIO_BUFFER_SIZE]
 * @brief Buffer which holds samples from last DMA interrupt
 */
uint16_t dmaAudioBuffer[AUDIO_BUFFER_SIZE];

/**
 * @var AmplitudeStr* mainSpectrumBuffer
 * @brief Buffer which holds spectrum samples
 */
SpectrumStr* mainSpectrumBuffer;

/* Task handlers */
osThreadId initTaskHandle;
osThreadDef(initTask, osPriorityNormal, 1,
		3*MINIMAL_STACK_SIZE);
		
#ifdef LCD_PRINTER_SUPPORT
osThreadId lcdTaskHandle;
osThreadDef(lcdTask, osPriorityNormal, 1, MINIMAL_STACK_SIZE);
#endif

osThreadId samplingTaskHandle;
osThreadDef(samplingTask, osPriorityRealtime, 1,
		2*MINIMAL_STACK_SIZE);

osThreadId streamingTaskHandle;
osThreadDef(streamingTask, osPriorityRealtime, 1,
		15*MINIMAL_STACK_SIZE);

osThreadId httpConfigTaskHandle;
osThreadDef(httpConfigTask, osPriorityHigh, 1,
		25*MINIMAL_STACK_SIZE);

osThreadId ethernetTaskHandle;
osThreadDef(ethernetTask, osPriorityNormal, 1,
		20*MINIMAL_STACK_SIZE);

osThreadId soundProcessingTaskHandle;
osThreadDef(soundProcessingTask, osPriorityHigh, 1,
		195*MINIMAL_STACK_SIZE);

/* Memory pool handlers */
osPoolDef(soundBufferPool, 1, SoundBufferStr);
osPoolId soundBufferPool_id;
osPoolDef(spectrumBufferPool, 2, SpectrumStr);
osPoolId spectrumBufferPool_id;
osPoolDef(cfftPool, 1, arm_cfft_instance_f32);
osPoolId cfftPool_id;
osPoolDef(soundProcessingBufferPool, 1,
		float32_t[MAIN_SOUND_BUFFER_MAX_BUFFER_SIZE]);
osPoolId soundProcessingBufferPool_id;

/* Mail queue handler */
osMailQDef(dmaAudioMail_q, MAXIMUM_DMA_AUDIO_MESSAGE_QUEUE_SIZE, SoundMailStr);
osMailQId dmaAudioMail_q_id;

/* Mutex handlers */
osMutexDef(mainSpectrumBufferMutex);
osMutexId mainSpectrumBufferMutex_id;

osMutexDef(ethernetInterfaceMutex);
osMutexId ethernetInterfaceMutex_id;

osMutexDef(mainSoundBufferMutex);
osMutexId mainSoundBufferMutex_id;

// FUNCTIONS

/**
 * @brief Function initializes the initialization task
 */
void threadsInit() {
	/* Starting OS task initialization */
	logMsg("Creating init task");
	initTaskHandle = osThreadCreate(osThread(initTask), NULL);
}

// OS TASKS

/**
 * @brief Initialization task
 */
void initTask(void const * argument) {
	/* PERIPHERALS INITIALIZATION */
	osEvent event;
	uint32_t i;
	
	lcdInit();
	logMsg("Init task");
	
	logMsg("Ethernet initialization...");
	ethInit();

	/* DHCP initialization */
	ethernetTaskHandle = osThreadCreate(osThread(ethernetTask), NULL);

	logMsg("Running ethernet thread");
	do {
		event = osSignalWait(ETHERNET_FINISHED_SIGNAL, osWaitForever);
	} while (event.status != osOK && event.status != osEventSignal);
	logMsg("Ethernet thread done");

	/* Taska, mutexes, mail queues and memory pools initialization */
	logMsg("Initializing memory pools");
	spectrumBufferPool_id = osPoolCreate(osPool(spectrumBufferPool));
	if (spectrumBufferPool_id == NULL)
		printNullHandle("Spect pool");
	cfftPool_id = osPoolCreate(osPool(cfftPool));
	if (cfftPool_id == NULL)
		printNullHandle("Cfft pool");
	soundBufferPool_id = osPoolCreate(osPool(soundBufferPool));
	if (soundBufferPool_id == NULL)
		printNullHandle("Sound pool");

	logMsg("Initializing mail queues");
	dmaAudioMail_q_id = osMailCreate(osMailQ(dmaAudioMail_q), NULL);
	if (dmaAudioMail_q_id == NULL)
		printNullHandle("Audio mail q");

	logMsg("Initializing mutexes");
	mainSpectrumBufferMutex_id = osMutexCreate(
			osMutex(mainSpectrumBufferMutex));
	if (mainSpectrumBufferMutex_id == NULL)
		printNullHandle("Spect mut");
	mainSoundBufferMutex_id = osMutexCreate(osMutex(mainSoundBufferMutex));
	if (mainSoundBufferMutex_id == NULL)
		printNullHandle("Audio mut");
	ethernetInterfaceMutex_id = osMutexCreate(osMutex(ethernetInterfaceMutex));
	if (ethernetInterfaceMutex_id == NULL)
		printNullHandle("Eth mut");

	/* Global variables */
	logMsg("Preparing global variables");
	configStr.amplitudeSamplingDelay = CONNECTION_TASK_DELAY_TIME;
	configStr.audioSamplingFrequency = 44100;
	configStr.clientPort = UDP_STREAMING_PORT;
	//strcpy(configStr.clientIp, UDP_STREAMING_IP);
	mainSpectrumBuffer = osPoolCAlloc(spectrumBufferPool_id);
	mainSoundBuffer = osPoolCAlloc(soundBufferPool_id);
	mainSoundBuffer->iterator = 0;
	mainSoundBuffer->frequency = configStr.audioSamplingFrequency;
	mainSoundBuffer->size = MAIN_SOUND_BUFFER_MAX_BUFFER_SIZE;
	for (i = 0; i < mainSoundBuffer->size; i++) {
		mainSoundBuffer->soundBuffer[i] = 0;
	}

	logMsg("Initializing tasks");
#ifdef LCD_PRINTER_SUPPORT
	lcdTaskHandle = osThreadCreate(osThread(lcdTask), NULL);
	if (lcdTaskHandle == NULL)
	printNullHandle("Lcd task");
#endif
	soundProcessingTaskHandle = osThreadCreate(osThread(soundProcessingTask), NULL);
	if (soundProcessingTaskHandle == NULL)
		printNullHandle("Sound proc task");
	samplingTaskHandle = osThreadCreate(osThread(samplingTask), NULL);
	if (samplingTaskHandle == NULL)
		printNullHandle("Samp task");
	streamingTaskHandle = osThreadCreate(osThread(streamingTask), NULL);
	if (streamingTaskHandle == NULL)
		printNullHandle("Stream task");
	httpConfigTaskHandle = osThreadCreate(osThread(httpConfigTask), NULL);
	if (httpConfigTaskHandle == NULL)
		printNullHandle("HTTP task");

	logMsg("Preparing audio recording");
	if (audioRecorderInit(AUDIO_RECORDER_INPUT_MICROPHONE, AUDIO_RECORDER_VOLUME_0DB, configStr.audioSamplingFrequency) != AUDIO_RECORDER_OK) {
		logErr("Audio rec init");
	}

	/* Audio recorder - start recording */
	if (audioRecorderStartRecording(dmaAudioBuffer,
	AUDIO_BUFFER_SIZE) != AUDIO_RECORDER_OK) {
		logErr("Audio buffer start");
	}
	
	logMsg("Terminating init");
	osThreadTerminate(initTaskHandle);
}

/**
 * @brief Ethernet task
 */
void ethernetTask(void const * argument) {
	uint32_t status;
	
	printIp();
	printNetmask();
	printGateway();

	// sending ethernet finished signal to initialization task
	status = osSignalSet(initTaskHandle, ETHERNET_FINISHED_SIGNAL);
	if (status != osOK) {
		logErrVal("ERROR: Eth finished signal", status);
	}
	
	logMsg("Executing net thread");
	while (1)
		net_main();
		osThreadYield();
}

void BSP_AUDIO_IN_Error_CallBack(void) {
	logMsg("Audio error callback");
}

/**
 * @brief Functions called as DMA interrupt
 */
void audioRecorder_FullBufferFilled(void) {
	SoundMailStr *soundSamples;
	osStatus mailStatus;
	
	// allocating memory for sound mail
	soundSamples = osMailAlloc(dmaAudioMail_q_id, 0);
	
	if(soundSamples == NULL)
	{
		logErr("Null sound samples");
	}
	else
	{
		audioRecordingSoundMailFill(soundSamples, dmaAudioBuffer, AUDIO_BUFFER_SIZE, configStr.audioSamplingFrequency);

		// sending mail to queue
		mailStatus = osMailPut(dmaAudioMail_q_id, soundSamples);
		if(mailStatus != osOK)
		{
			logErrVal("DMA irq ", mailStatus);
		}
	}
}

/**
 * @brief Asynchronous task which gets audio mails from queue and fills the mainSoundBuffer
 */
void samplingTask(void const * argument) {
	osStatus status;
	osEvent event;
	SoundMailStr *receivedSound;
	
	while (1) {
		// waiting for new mail
		osThreadYield();
		event = osMailGet(dmaAudioMail_q_id, osWaitForever);
		if (event.status == osEventMail) {
			receivedSound = (SoundMailStr *) event.value.p;
		
			// waiting for access to mailSoundBuffer
			status = osMutexWait(mainSoundBufferMutex_id, osWaitForever);
			if (status == osOK) {
				// filling cyclic buffer
				audioRecordingUpdateSoundBuffer(mainSoundBuffer, receivedSound);

				// releasing mutex
				status = osMutexRelease(mainSoundBufferMutex_id);
				if (status != osOK) {
					logErrVal("Sampling mutex release", status);
				}
			} else {
				logErr("Sampling mutex");
			}

			// free audio mail memory
			status = osMailFree(dmaAudioMail_q_id, receivedSound);
			if (status != osOK) {
				logErrVal("Sound mail free", status);
			}
		}
	}
}

/**
 * @brief FFT processing task
 */
void soundProcessingTask(void const * argument) {
	SpectrumStr* temporarySpectrumBufferStr;
	arm_cfft_instance_f32* cfftInstance;
	osStatus status;

	osEvent event;
	float32_t temporaryAudioBuffer[MAIN_SOUND_BUFFER_MAX_BUFFER_SIZE];

	// allocating memory for temporary spectrum buffer
	temporarySpectrumBufferStr = osPoolCAlloc(spectrumBufferPool_id);
	cfftInstance = osPoolCAlloc(cfftPool_id);

	while (1) {	
		// waiting for start signal
		event = osSignalWait(START_SOUND_PROCESSING_SIGNAL, osWaitForever);

		if (event.status == osEventSignal) {

			// waiting for access to main sound buffer
			status = osMutexWait(mainSoundBufferMutex_id, osWaitForever);
			if (status == osOK) {

				// getting FFT instance
				soundProcessingGetCfftInstance(cfftInstance,
						mainSoundBuffer->size / 2);

				if (cfftInstance != NULL) {

					// spectrum buffer initialization and sound buffer copying
					soundProcessingAmplitudeInit(temporarySpectrumBufferStr,
							mainSoundBuffer, temporaryAudioBuffer);

					// releasing mainSoundBufferMutex
					status = osMutexRelease(mainSoundBufferMutex_id);
					if (status != osOK) {
						logErrVal("Sampling mutex (sound processing) release",
								status);
					}

					// calculating spectrum
					soundProcessingGetAmplitudeInstance(cfftInstance,
							temporarySpectrumBufferStr, temporaryAudioBuffer);

					// waiting for access to main spectrum buffer
					status = osMutexWait(mainSpectrumBufferMutex_id,
					osWaitForever);
					if (status == osOK) {

						// copying spectrum from temporary buffer to main buffer
						soundProcessingCopyAmplitudeInstance(
								temporarySpectrumBufferStr, mainSpectrumBuffer);
						
						// releasing main spectrum buffer mutex
						status = osMutexRelease(mainSpectrumBufferMutex_id);
						if (status != osOK) {
							logErrVal("Shared amp mutex released", status);
						}
					} else {
						logErrVal("Shared amp mutex wait", status);
					}

				} else {
					logErr("Cfft NULL");

					// releasing main sound buffer mutex
					status = osMutexRelease(mainSoundBufferMutex_id);
					if (status != osOK) {
						logErrVal("Sampling mutex (sound processing) release",
								status);
					}
				}
			} else {
				logErr("Sampling mutex (sound processing)");
			}
		} else
		logErrVal("ST sp wait", event.status);
	}
}

#ifdef LCD_PRINTER_SUPPORT
void lcdTask(void const * argument) {
	while (1) {
		osDelay(LCD_TASK_DELAY_TIME);
		osStatus status = osMutexWait(mainSpectrumBufferMutex_id, osWaitForever);
		if (status == osOK) {
			lcdAmpPrinterPrint(&mainSpectrumBuffer);
			status = osMutexRelease(mainSpectrumBufferMutex_id);
			if (status != osOK) {
				logErrVal("lcdFrequencyMutex release", status);
			}
		}
	}
}
#endif

/**
 * @brief Spectrum UDP streaming
 */
void streamingTask(void const * argument) {
	int32_t status;
	netStatus netStatusVal;
	
	initStreamingSocket();	
	
	while (1) {
		// setting signal to start sound processing
		status = osSignalSet(soundProcessingTaskHandle, START_SOUND_PROCESSING_SIGNAL);
		
		if(netStatusVal == netOK)
		{
			osDelay(configStr.amplitudeSamplingDelay);
		}
		else
		{
			osDelay(UDP_STREAMING_FAILURE_TIMEOUT);
		}

		// waiting for acces to ethernet interface
		status = osMutexWait(ethernetInterfaceMutex_id, osWaitForever);
		if (status == osOK) {
				// "connecting" to UDP
				openStreamingSocket(configStr.clientPort);
				
				// sending main spectrum buffer by UDP
				netStatusVal = sendSpectrum(mainSpectrumBuffer, "IP", configStr.clientPort);
				if(netStatusVal != netOK) {
					logErrVal("Net status ", netStatusVal);
				}
				
				closeStreamingSocket();
		}
	}
}

/**
 * @brief Device configuration (by network using HTTP)
 */
void httpConfigTask(void const* argument) {
	osEvent event;
	char data[1024];
	int32_t httpSocket;
	
	initHttpSocket(&httpConfigTaskHandle);
	httpStartListen();
	
	while(1)
	{
		event = osSignalWait(GET_REQUEST_SIGNAL, osWaitForever);
		if (event.status == osOK || event.status == osEventSignal)
		{
			// waiting for finish the socket interrupt
			osDelay(1);
			
			httpSocket = getHttpSocket();
			getData(data);
			
			if(isConfigRequest(data))
			{
				sendConfiguration(&configStr, httpSocket, "\r\nConnection: Closed");
			}
			else if(isSystemRequest(data))
			{
				sendHttpResponse(httpSocket, "501 Not Implemented","\r\nContent-Type: text/html", "<h1>System info not supported on RTX</h1>");
			}
			else
			{
				sendHttpResponse(httpSocket, "404 Not Found", "\r\nContent-Type: text/html", "<h1>404 Not Found</h1>");
			}
			
			closeSocket(httpSocket);
		}
	}
}
