/*
 * uartecho.c
 *
 *  Created on: Dec 1, 2023
 *      Author: RISHIKESH
 */
/*
 * Glo.uartEcho.c
 *
 *  Created on: Sep 17, 2022
 *      Author: root
 */

#include "mainHead.h"

void *taskUartEcho(void *arg0)
{
    int indexCommand = 0;

    UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    UART_write(Glo.uart, Glo.var.echoPrompt, strlen(Glo.var.echoPrompt));
    UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    /* Loop forever echoing */
    while (1) {
        Glo.var.arrowInput[0] = 0;
        UART_read(Glo.uart, &Glo.var.input, 1);
        if(Glo.var.input == '\33') {
            UART_read(Glo.uart, Glo.var.arrowInput, 2);
            if(commandTest("[A", Glo.var.arrowInput)){
                UART_write(Glo.uart, "\r", strlen("\r"));
                UART_write(Glo.uart, "                                    ", strlen("                                    "));
                UART_write(Glo.uart, "\r", strlen("\r"));
                UART_write(Glo.uart, Glo.var.previousCommand, strlen(Glo.var.previousCommand));
                strcpy(Glo.var.command, Glo.var.previousCommand);
                indexCommand = strlen(Glo.var.command);
                continue;
            }
            else{
                UART_write(Glo.uart, &Glo.var.input, 1);
                UART_write(Glo.uart, &Glo.var.arrowInput, 2);
            }
        }
        if(indexCommand < 127){
            UART_write(Glo.uart, &Glo.var.input, 1);
        }

        if(Glo.var.input == '\r') {
            addMessage(Glo.var.command);
            Semaphore_post(Glo.msgQueSem);

            strcpy(Glo.var.previousCommand, Glo.var.command);
            memset(Glo.var.command, 0, strlen(Glo.var.command));
            indexCommand=0;
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            Glo.var.overflowFlag = 0;
        }
        else if (Glo.var.input == '\b' || Glo.var.input == 0x7F && indexCommand > 0){
            indexCommand--;
            Glo.var.command[indexCommand] = 0;
            UART_write(Glo.uart, " \b", strlen(" \b"));
        }
        else if (indexCommand < 127) {
            Glo.var.command[indexCommand] = Glo.var.input;
            indexCommand++;
        }
        if (indexCommand == 127 && Glo.var.overflowFlag == 0) {
            Glo.var.overflowFlag = 1;
            Glo.var.errorCount[0]++;
        }
    }
}







