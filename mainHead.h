/*
 * mainHead.h
 *
 *  Created on: Sep 18, 2022
 *      Author: root
 */

#ifndef MAINHEAD_H_
#define MAINHEAD_H_

#define TICKERNUM   16
#define REGISTERNUM 32
#define SCRIPTNUM   64
#define QUELEN      16
#define MESSAGELEN 320
#define ADCBUFLEN   12
#define VOICELEN   128
#define SHADOW_NET_IP 0
#define DEFAULT_NET_PORT 1000

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/ADC.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "ti_drivers_config.h"

typedef struct variables{
    char *aboutOutput;
    int messageIndex;
    char *newLine;
    char        input;
    const char *echoPrompt;

    const char *helpHelpOutput;
    const char *helpAboutOutput;
    const char *helpPrintOutput;
    const char *helpMemrOutput;
    const char *helpGPIOOutput;
    const char *helpErrorOutput;
    const char *helpTimerOutput;
    const char *helpCallbackOutput;
    const char *helpTickerOutput;
    const char *helpRegOutput;
    const char *helpScriptOutput;
    const char *helpIfOutput;
    const char *helpUartOutput;
    const char *helpSineOutput;
    const char *helpNetudpOutput;
    const char *helpAudioOutput;

    const char *help;
    const char *about;
    char *command;
    char *previousCommand;
    char *callbackCommand;
    char *uart7readBuffer;
    int i;
    char *arrowInput;
    // overflow, incorrect command, invalid mem address
    int errorCount[3];
    int overflowFlag;
} Variables;

typedef struct _sine{
    SPI_Handle spi;
    uint16_t *sineVal;
    SPI_Transaction spiTransaction;
    double sineIndex;
    double indexJump;
    double remainder;
    uint16_t outVal;
} Sine;

typedef struct _callbacks{
    char payload[MESSAGELEN];
    int callbackCount;
    int period;
} Callback;

typedef struct _tickers{
    char payload[MESSAGELEN];
    int tickerCount;
    int delay;
    int period;
} Ticker;

typedef struct _message{
    char message[MESSAGELEN]; //allocate memory
} Message;

typedef struct _messageQueue{
    Message messages[QUELEN];
    int readIndex;
    int writeIndex;
} MessageQueue;

typedef struct _script{
    char payload[MESSAGELEN];
} Script;

typedef struct _udp{
    char ip[MESSAGELEN];
    char port[MESSAGELEN];
    char payload[MESSAGELEN];
    Semaphore_Handle sem;
} UDP;

typedef struct _adc{
    ADCBuf_Handle bufferHandle;
    ADC_Handle handle;
    bool audioOn;
    uint16_t ping[VOICELEN];
    uint16_t pong[VOICELEN];
    bool pp;
} ADC;

typedef struct _voice{
    uint16_t *buffer;
    int_fast16_t index;
    bool voiceIn;
    bool voiceOut;
    uint16_t ping[VOICELEN];
    uint16_t pong[VOICELEN];
    bool pp;
} Voice;

typedef struct _globals {
    Timer_Handle timer0;
    Timer_Handle timer1;
    UART_Handle uart;
    UART_Handle uart7;
    Variables var;
    MessageQueue msgQue;
    Callback callback[3];
    Ticker ticker[TICKERNUM];
    Semaphore_Handle msgQueSem;
    int32_t reg[REGISTERNUM];
    Script script[SCRIPTNUM];
    Sine sine;
    UDP udp;
    ADC adc;
    Voice voice;
} Globals;

#ifndef MAIN
extern
#endif
Globals Glo;

void timerCallback(Timer_Handle myHandle, int_fast16_t status);
void tickerCallback(Timer_Handle myHandle, int_fast16_t status);

void adcCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *buffer, uint32_t channel, int_fast16_t status);
void addMessage(const char *inMessage);
void commandEntry(char *command);
int commandTest(const char *command, const char *compareString);
int indexOf(const char *string, const char character);
void infra(void);
void initializeDrivers(void);
void leftButtonCallback(void);
void rightButtonCallback(void);
char *secondString(char *fullString);
void stringCopy(char *outString, const char *copiedString);
void taskCommandServicer(void);

void *udpReceive(void *);
void *udpSend(void *);

void audioFoo(uint16_t convBuffer);

#endif /* MAINHEAD_H_ */
