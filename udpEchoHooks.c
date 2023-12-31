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
 *    ======== udpEchoHooks.c ========
 */

#include <mainHead.h>

#include <pthread.h>

#include <ti/ndk/inc/netmain.h>

#include <ti/ndk/slnetif/slnetifndk.h>
#include <ti/net/slnet.h>
#include <ti/net/slnetif.h>
#include <ti/net/slnetutils.h>

#include <ti/drivers/emac/EMACMSP432E4.h>

#define UDPPORT 1000

#define UDPHANDLERSTACK 4096
#define IFPRI  4   /* Ethernet interface priority */

/* Prototypes */
extern void *echoFxn(void *arg0);

/*
 *  ======== netIPAddrHook ========
 *  user defined network IP address hook
 */
void netIPAddrHook(uint32_t IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    pthread_t          thread;
    pthread_attr_t     attrs;
    struct sched_param priParam;
    int                retc;
    int                detachState;
    uint32_t           hostByteAddr;
    static uint16_t    arg0 = UDPPORT;
    static bool        createTask = true;
    int32_t            status = 0;

    if (fAdd) {
        UART_write(Glo.uart, "\r\nNetwork Added: \r\n", strlen("\r\nNetwork Added: \r\n"));
    }
    else {
        UART_write(Glo.uart, "\r\nNetwork Removed: \r\n", strlen("\r\nNetwork Removed: \r\n"));
    }

    /* print the IP address that was added/removed */
    hostByteAddr = NDK_ntohl(IPAddr);
    char hostByteAddrBuffer[MESSAGELEN];
    sprintf(hostByteAddrBuffer, "\r\nIf-%d:%d.%d.%d.%d\r\n", IfIdx,
            (uint8_t)(hostByteAddr>>24)&0xFF, (uint8_t)(hostByteAddr>>16)&0xFF,
            (uint8_t)(hostByteAddr>>8)&0xFF, (uint8_t)hostByteAddr&0xFF);

    UART_write(Glo.uart, hostByteAddrBuffer, strlen(hostByteAddrBuffer));

    /* initialize SlNet interface(s) */
    status = ti_net_SlNet_initConfig();
    if (status < 0)
    {
        char statusBuffer[MESSAGELEN];
        sprintf(statusBuffer, "\r\nFailed to initialize SlNet interface(s)\r\n- status (%d)\r\n", status);
        UART_write(Glo.uart, statusBuffer, strlen(statusBuffer));
//        while (1);
    }

    if (fAdd && createTask) {
        /*
         *  Create the Task that handles incoming UDP packets.
         *  arg0 will be the port that this task listens to.
         */

        /* Set priority and stack size attributes */
        pthread_attr_init(&attrs);
        priParam.sched_priority = 1;

        detachState = PTHREAD_CREATE_DETACHED;
        retc = pthread_attr_setdetachstate(&attrs, detachState);
        if (retc != 0) {
            UART_write(Glo.uart, "\r\nnetIPAddrHook: pthread_attr_setdetachstate() failed\r\n", strlen("\r\nnetIPAddrHook: pthread_attr_setdetachstate() failed\r\n"));
//            while (1);
        }

        pthread_attr_setschedparam(&attrs, &priParam);

        retc |= pthread_attr_setstacksize(&attrs, UDPHANDLERSTACK);
        if (retc != 0) {
            UART_write(Glo.uart, "\r\nnetIPAddrHook: pthread_attr_setstacksize() failed\r\n", strlen("\r\nnetIPAddrHook: pthread_attr_setstacksize() failed\r\n"));
//            while (1);
        }

        retc = pthread_create(&thread, &attrs, udpReceive, (void *)&arg0);
        if (retc != 0) {
            UART_write(Glo.uart, "\r\nnetIPAddrHook: pthreadReceive_create() failed\r\n", strlen("\r\nnetIPAddrHook: pthread_create() failed\r\n"));
//            while (1);
        }

        retc = pthread_create(&thread, &attrs, udpSend, (void *)&arg0);
        if (retc != 0) {
            UART_write(Glo.uart, "\r\nnetIPAddrHook: pthreadSend_create() failed\r\n", strlen("\r\nnetIPAddrHook: pthread_create() failed\r\n"));
//            while (1);
        }

        createTask = false;
    }
}

/*
 *  ======== serviceReport ========
 *  NDK service report.  Initially, just reports common system issues.
 */
void serviceReport(uint32_t item, uint32_t status, uint32_t report, void *h)
{
    static char *taskName[] = {"Telnet", "", "NAT", "DHCPS", "DHCPC", "DNS"};
    static char *reportStr[] = {"", "Running", "Updated", "Complete", "Fault"};
    static char *statusStr[] =
        {"Disabled", "Waiting", "IPTerm", "Failed","Enabled"};
    char statusRepBuffer[MESSAGELEN];

    sprintf(statusRepBuffer, "\r\nService Status: %-9s: %-9s: %-9s: %03d\r\n",
            taskName[item - 1], statusStr[status], reportStr[report / 256],
            report & 0xFF);
    UART_write(Glo.uart, statusRepBuffer, strlen(statusRepBuffer));

    /* report common system issues */
    if ((item == CFGITEM_SERVICE_DHCPCLIENT) &&
            (status == CIS_SRV_STATUS_ENABLED) &&
            (report & NETTOOLS_STAT_FAULT)) {
        UART_write(Glo.uart, "\r\nDHCP Client initialization failed; check your network.\r\n", strlen("\r\nDHCP Client initialization failed; check your network.\r\n"));
//        while (1);
    }
}

