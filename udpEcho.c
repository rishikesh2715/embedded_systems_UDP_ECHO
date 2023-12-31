/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *    ======== udpEcho.c ========
 *    Contains BSD sockets code.
 */

#include <mainHead.h>

#include <pthread.h>
/* BSD support */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <ti/net/slnetutils.h>

#define UDPPACKETSIZE 1472
#define MAXPORTLEN    6

extern void fdOpenSession();
extern void fdCloseSession();
extern void *TaskSelf();
/*
 *  ======== echoFxn ========
 *  Echoes UDP messages.
 *
 */
void *udpReceive(void *arg0)
{
    int                bytesRcvd;
    int                status;
    int                server = -1;
    fd_set             readSet;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               buffer[UDPPACKETSIZE];
    char               portNumber[MAXPORTLEN];
    char               outputMsg[320];

    fdOpenSession(TaskSelf());

    UART_write(Glo.uart, "\r\nUDP Echo example starter\r\n", strlen("\r\nUDP Echo example started\r\n"));

    sprintf(portNumber, "%d", *(uint16_t *)arg0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
        char gaiErrorBuffer[50];
        sprintf(gaiErrorBuffer, "\r\nError: getaddrinfo() failed: %s\r\n", gai_strerror(status));
        UART_write(Glo.uart, gaiErrorBuffer, strlen(gaiErrorBuffer));
        goto shutdown;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server == -1) {
            continue;
        }

        status = bind(server, p->ai_addr, p->ai_addrlen);
        if (status != -1) {
            break;
        }

        close(server);
    }

    if (server == -1) {
        UART_write(Glo.uart, "\r\nError: socket not created.\r\n", strlen("\r\nError: socket not created.\r\n"));
        goto shutdown;
    } else if (p == NULL) {
        UART_write(Glo.uart, "\r\nError: bind failed.\r\n", strlen("\r\nError: bind failed.\r\n"));
        goto shutdown;
    } else {
        freeaddrinfo(res);
        res = NULL;
    }

    do {
        /*
         *  readSet and addrlen are value-result arguments, which must be reset
         *  in between each select() and recvfrom() call
         */
        FD_ZERO(&readSet);
        FD_SET(server, &readSet);
        addrlen = sizeof(clientAddr);

        /* Wait forever for the reply */
        status = select(server + 1, &readSet, NULL, NULL, NULL);
        if (status > 0) {
            if (FD_ISSET(server, &readSet)) {
                bytesRcvd = recvfrom(server, buffer, UDPPACKETSIZE, 0,
                        (struct sockaddr *)&clientAddr, &addrlen);
                // 0 end of string
                if(bytesRcvd > 0)
                {
                    char msgBuf[MESSAGELEN];
                    memcpy(msgBuf, buffer, bytesRcvd);
                    msgBuf[bytesRcvd] = 0;
                    if(!commandTest("-voice", buffer)){
                        UART_write(Glo.uart, msgBuf, strlen(msgBuf));
                        addMessage(msgBuf);
                        Semaphore_post(Glo.msgQueSem);
                    }
                    else {
                        Glo.voice.pp = atoi(secondString(msgBuf));
                        if(!Glo.voice.pp) memcpy(Glo.voice.ping, &msgBuf[strlen(msgBuf) + 1], sizeof(uint16_t) * VOICELEN);
                        else memcpy(Glo.voice.pong, &msgBuf[strlen(msgBuf) + 1], sizeof(uint16_t) * VOICELEN);
                    }
                }
            }
        }
    } while (status > 0);

shutdown:
    if (res) {
        freeaddrinfo(res);
    }

    if (server != -1) {
        close(server);
    }

    fdCloseSession(TaskSelf());

    return (NULL);
}

void *udpSend(void *arg0)
{
    int                bytesSent;
    int                status;
    int                server = -1;
    struct addrinfo    hints;
    struct addrinfo    *res, *p;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               portNumber[MAXPORTLEN];

    fdOpenSession(TaskSelf());

    UART_write(Glo.uart, "\r\nUDP Echo example starter\r\n", strlen("\r\nUDP Echo example started\r\n"));

    sprintf(portNumber, "%d", *(uint16_t *)arg0);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    /* Obtain addresses suitable for binding to */
    status = getaddrinfo(NULL, portNumber, &hints, &res);
    if (status != 0) {
        char gaiErrorBuffer[50];
        sprintf(gaiErrorBuffer, "\r\nError: getaddrinfo() failed: %s\r\n", gai_strerror(status));
        UART_write(Glo.uart, gaiErrorBuffer, strlen(gaiErrorBuffer));
        goto shutdown;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server == -1) {
            continue;
        }

        status = bind(server, p->ai_addr, p->ai_addrlen);
        if (status != -1) {
           break;
        }

       close(server);
    }

    if (server == -1) {
        UART_write(Glo.uart, "\r\nError: socket not created.\r\n", strlen("\r\nError: socket not created.\r\n"));
        goto shutdown;
    }
    else {
        freeaddrinfo(res);
        res = NULL;
    }

//    status = 1;

    while(1){
        /*
         *  readSet and addrlen are value-result arguments, which must be reset
         *  in between each select() and recvfrom() call
         */

//        FD_ZERO(&readSet);
//        FD_SET(server, &readSet);
        addrlen = sizeof(clientAddr);

        Semaphore_pend(Glo.udp.sem, BIOS_WAIT_FOREVER);

        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = htons(atoi(Glo.udp.port));
        clientAddr.sin_addr.s_addr = inet_addr(Glo.udp.ip);

        //len = strlen + 1 for 0 +256
        int len = 0;
        len = strlen(Glo.udp.payload);
        if(commandTest("-voice", Glo.udp.payload)) len += 257;

        bytesSent = sendto(server, Glo.udp.payload, len, 0, (struct sockaddr *)&clientAddr, addrlen);
        if (bytesSent < 0) {
            addMessage("-print message send failed");
            Semaphore_post(Glo.msgQueSem);
            goto shutdown;
        }
        else {
            addMessage("-print message sent");
            Semaphore_post(Glo.msgQueSem);
        }
    };

shutdown:
    if (res) {
        freeaddrinfo(res);
    }

    if (server != -1) {
        close(server);
    }

    fdCloseSession(TaskSelf());

    return (NULL);
}


