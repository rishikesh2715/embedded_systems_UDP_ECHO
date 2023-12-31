/*
 * infrastructure.c
 *
 *  Created on: Dec 1, 2023
 *      Author: RISHIKESH
 */


/*
 * infrastructure.c
 *
 *  Created on: Sep 26, 2022
 *      Author: root
 */

#include <mainHead.h>

char aboutOutput[MESSAGELEN] = "";
int timerCallbackCount = 0;
int leftButtonCallbackCount = 0;
int rightButtonCallbackCount = 0;
int messageIndex = 0;
char newLine[2] = "\r\n";
extern Semaphore_Handle semaphore0;
extern Semaphore_Handle semaphore1;

char        input;

const char echoPrompt[]         = "\r\nWelcome to MSP432:\r\n";

const char helpHelpOutput[]     = "\r\n-help prints information on available commands. -help command for more detailed information on that command."
                                  "\r\n\tEx: -help print. Prints the help string for the print command.\r\n";

const char helpAboutOutput[]    = "\r\n-about prints the name, assignment number, version and date/time of compile\r\n";

const char helpPrintOutput[]    = "\r\n-print prints any text following the command"
                                  "\r\n\tEx: -print hello world. Prints 'hello world'.\r\n";

const char helpMemrOutput[]     = "\r\n-memr shows what is in the memory space specified after the command"
                                  "\r\n\tFlash: 0x00000000 to 0x000FFFFF"
                                  "\r\n\tSRAM: 0x20000000 to 0x2003FFFF"
                                  "\r\n\tPeripherals: 0x40000000 to 0x44055000"
                                  "\r\n\tEx: -memr 0x2. Prints the memory contents of 0 through F.\r\n";

const char helpGPIOOutput[]     = "\r\n-gpio allows to you read (-r), toggle (-t), and write (-w) to the GPIO pins."
                                  "\r\n\tThe first argument is the pin number, second argument is the command type (-t, -w, -r),"
                                  "\r\n\tand the third is 1 or 0 if you are writing high or low"
                                  "\r\n\t0-3 are LEDs 1-4, 4 and 5 are the left and right buttons, 6 is PK5 and 7 is PD4(*RX)."
                                  "\r\n\tWhen connected to the BOOSTXL-AUDIO PK5 (GPIO 6) set to 0 (low) turns the amp on, while setting PD4(*RX) to 1 turns the mic on."
                                  "\r\n\tEx: -gpio 0 -w 1. Turns on GPIO 1.\r\n";

const char helpErrorOutput[]    = "\r\n-error shows the current count of errors.\r\n";

const char helpTimerOutput[]    = "\r\n-timer gets and sets the current timer period. With no argument it gets."
                                  "\r\n\tWith a number above 100 following the command the timer is set to that number in microseconds."
                                  "\r\n\tEx: -timer 1000. The Callback 0 timer period will be set to 1000 us.\r\n";

const char helpCallbackOutput[] = "\r\n-callback gets and sets callback commands for a timer and the two side buttons."
                                  "\r\n\tThe first argument is which callback (0 = timer, 1 = left button, 2 = right button)."
                                  "\r\n\tThe second argument is the number of callbacks (-1 is infinite)."
                                  "\r\n\tThe third argument is the command that you want to be called (Ex. -gpio 0 -t).\r\n"
                                  "\r\n\tIf you use no arguments then it will print all of the commands currently in the callback functions."
                                  "\r\n\tIf you only use one argument then it will print the command currently in that callback functions."
                                  "\r\n\tEx: -callback 1 3 -reg 0 ++ 0."
                                  "\r\n\tRegister 0 will increment each time the left button (Callback 1) is pressed up to 3 times.\r\n";

const char helpTickerOutput[]   = "\r\n-ticker gets and sets callback commands for 16 tickers that call every 10 ms."
                                  "\r\n\tThe first argument is which of the 16 tickers to use."
                                  "\r\n\tThe second argument is the delay before the ticker callback starts in multiples of 10 ms."
                                  "\r\n\tThe third argument is the period between ticker calls."
                                  "\r\n\tThe fourth argument is the number of callbacks called in total (-1 = infinite)."
                                  "\r\n\tThe fifth argument is the command that you would like to be called."
                                  "\r\n\tIf you call ticker with no arguments it prints the commands in each ticker."
                                  "\r\n\tIf you only specify the first argument then it prints the command for that index."
                                  "\r\n\tEx: -ticker 0 100 100 3 -print banana."
                                  "\r\n\tTicker 0 will wait 100*10ms and then print banana 3 times with 100*10ms in between each print.\r\n";

const char helpRegOutput[]      = "\r\n-reg gets and sets 32 registers and allows you to do basic instructions on them."
                                  "\r\n\tThe first argument is which of the 32 registers to input the result into."
                                  "\r\n\tThe second argument is the instruction you wish to use."
                                  "\r\n\tThe third argument is the first operand involved in the instruction (usually a register)."
                                  "\r\n\tThe fourth argument is the second register involved in the operation."
                                  "\r\n\tCalling -reg with no arguments prints the contents of all 32 registers."
                                  "\r\n\tCalling -reg with one argument prints the contents of that register."
                                  "\r\n\tThe instructions are as follows:"
                                  "\r\n\tWrite      (=) puts the immediate value in the first operand into the register."
                                  "\r\n\tIncrement (++) increments the value in the first operand register and puts the result in the input register."
                                  "\r\n\tDecrement (--) decrements the value in the first operand register and puts the result in the input register."
                                  "\r\n\tAdd        (+) adds the values in the first and second operand register and puts the result in the input register."
                                  "\r\n\tSubtract   (-, two operands) subtracts the value in the second operand register from the value in the first operand register and puts the result in the input register."
                                  "\r\n\tNegative   (-, one operand) get the negative value of the first operand register and puts the value in the input register"
                                  "\r\n\tNot        (~) bitwise not the first operand register and put the result in the input register."
                                  "\r\n\tAnd        (&) bitwise and the first operand register and the second operand register and put the result in the input register."
                                  "\r\n\tOr         (|) bitwise or the first operand register and the second operand regsiter and put the result in the input register."
                                  "\r\n\tXor        (^) bitwise xor the first operand register and the second operand register and put the result in the input register."
                                  "\r\n\tMultiply   (*) multiply the first and second operand register and put the result in the input register."
                                  "\r\n\tDivide     (/) divide the first operand register by the second operand register and put the result in the input register."
                                  "\r\n\tRemainder  (%) get the remainder from the division of the first operand register by the second and put the result in the input register."
                                  "\r\n\tMax        (>) put the higher value from the first and second operand register into the input register."
                                  "\r\n\tMin        (<) put the lower of the first and second operand register values into the input register."
                                  "\r\n\tCopy       (c) copy the value of the first operand register into the input register."
                                  "\r\n\tExchange   (x) exchange the values of the input register and the first operand register."
                                  "\r\n\tEx: -reg 0 + 1 #3. Register 0 gets the result of adding 3 to the value in register 1.\r\n";

const char helpScriptOutput[]   = "\r\n-script gets and sets 64 lines of a script in which you can put commands that will be executed simultaneously."
                                  "\r\n\tExecuting a script will execute the line specified and all lines after until a blank line or the last line is reached."
                                  "\r\n\tThe first argument is the mode flag: -x (execute), -w (write), -r (read)."
                                  "\r\n\tThe second argument is the line number you wish to operate on."
                                  "\r\n\tThe third argument is the command you wish to write."
                                  "\r\n\tCalling -script with no arguments or only -r prints the contents of all 32 lines."
                                  "\r\n\tCalling -script -x will execute all lines starting from 0 to the next blank line or the end."
                                  "\r\n\tEx: -script -x 4. Executes line 4 and all lines after until a blank line or the end."
                                  "\r\n\tScript 0-3 is a conditional test you can call with -script 2."
                                  "\r\n\tScript 6-8 turns on audio output."
//                                  "\r\n\tScript 10-13 turns on audio input."
                                  "\r\n\tScript 15-16 turns off audio output.\r\n";
//                                  "\r\n\tScript 18-20 turns off audio input.\r\n";

const char helpIfOutput[]       = "\r\n-if allows you to compare two values and execute commands based on the result."
                                  "\r\n\tThe values can be immediate (# in front) or registers."
                                  "\r\n\tThere is three types of conditionals you can use: less than (<), greater than (>), or equal to (==)."
                                  "\r\n\tThe command before the colon is the command executed if the conditional is true and the command after is executed if the conditional is false."
                                  "\r\n\tThe first argument is the first value."
                                  "\r\n\tThe second argument is the conditional."
                                  "\r\n\tThe third argument is the second value."
                                  "\r\n\tThe fourth argument is a '?' which separates the conditional from the results."
                                  "\r\n\tThe fifth argument is the command if true."
                                  "\r\n\tThe sixth argument is a ':' which separates the results."
                                  "\r\n\tThe seventh argument is the command if false."
                                  "\r\n\tEx: -if 4 > #3 ? -gpio 0 -t : -gpio 1 -t."
                                  "\r\n\tIf the value in register 4 is greater than 3 then GPIO 0 will toggle, otherwise GPIO 1 will toggle.\r\n";

const char helpUartOutput[]     = "\r\n-uart sends the payload out of UART 7."
                                  "\r\n\tUART 7 corresponds to TX pin PC5 and RX pin PC4."
                                  "\r\n\tThe only argument is the payload."
                                  "\r\n\tEx: -uart -print hello."
                                  "\r\n\tWill send '-print hello' out of UART 7.\r\n";

const char helpSineOutput[]     = "\r\n-sine outputs a tone of the specified frequency from the BOOSTXL-AUDIO board."
                                  "\r\n\tThe only argument is the frequency."
                                  "\r\n\tSetting the frequency to 0 will turn off sine."
                                  "\r\n\tEx: -sine 220."
                                  "\r\n\tPlays a tone with 220 Hz frequency (A note).\r\n";

const char helpNetudpOutput[]   = "\r\n-netudp sends a payload to the specified IP and port number."
                                  "\r\n\tThe first argument is the IP address and port number to send to separated by a colon."
                                  "\r\n\tThe second argument is the payload."
                                  "\r\n\tEx: -netudp 10.0.0.6:1000 -print hello."
                                  "\r\n\tSends '-print hello' to the IP Address 10.0.0.6, port 1000.\r\n";

const char helpAudioOutput[]    = "\r\n-audio turns on the mic and starts playing the audio out of the speaker on the audio board."
                                  "\r\n\tThere are no arguments, entering the command toggles the functionality on and off."
                                  "\r\n\tInitital state is off."
                                  "\r\n\tEx: -audio."
                                  "\r\n\tTurns on mic and starts playing audio if in initial state, turns off mic otherwise.\r\n";

const char help[] = "-help";
const char about[] = "-about";
char command[MESSAGELEN] = "";
char previousCommand[MESSAGELEN] = "";
char callbackCommand[MESSAGELEN] = "";
int i = 0;
char arrowInput[2] = "";
// overflow, incorrect command, invalid mem address
int errorCount[] = {0, 0, 0};
int overflowFlag = 0;
int callbackPeriod = 1000000;
uint16_t sineBuffer;
extern uint16_t sineVal[256];
double sineIndex = 0;
double indexJump = 4;
double sineRemainder = 0;
uint16_t outVal = 0;

void addMessage(const char *inMessage){
    stringCopy(Glo.msgQue.messages[Glo.msgQue.writeIndex].message, inMessage);
    Glo.msgQue.writeIndex++;
    if(Glo.msgQue.writeIndex == QUELEN) Glo.msgQue.writeIndex = 0;
}

void audioFoo(uint16_t convBuffer){
    SPI_Transaction spiTransaction;

    spiTransaction.count = 1;
    spiTransaction.txBuf = (void *) &convBuffer;
    spiTransaction.rxBuf = (void *) NULL;
    SPI_transfer(Glo.sine.spi, &spiTransaction);
}

void commandEntry(char *command) {
    if(commandTest(help, command)){
        char *helpCommand;
        helpCommand = secondString(command);

        if(strstr(helpCommand, "help")) UART_write(Glo.uart, Glo.var.helpHelpOutput, strlen(Glo.var.helpHelpOutput));
        else if(strstr(helpCommand, "about")) UART_write(Glo.uart, Glo.var.helpAboutOutput, strlen(Glo.var.helpAboutOutput));
        else if(strstr(helpCommand, "print")) UART_write(Glo.uart, Glo.var.helpPrintOutput, strlen(Glo.var.helpPrintOutput));
        else if(strstr(helpCommand, "memr")) UART_write(Glo.uart, Glo.var.helpMemrOutput, strlen(Glo.var.helpMemrOutput));
        else if(strstr(helpCommand, "gpio")) UART_write(Glo.uart, Glo.var.helpGPIOOutput, strlen(Glo.var.helpGPIOOutput));
        else if(strstr(helpCommand, "error")) UART_write(Glo.uart, Glo.var.helpErrorOutput, strlen(Glo.var.helpErrorOutput));
        else if(strstr(helpCommand, "timer")) UART_write(Glo.uart, Glo.var.helpTimerOutput, strlen(Glo.var.helpTimerOutput));
        else if(strstr(helpCommand, "callback")) UART_write(Glo.uart, Glo.var.helpCallbackOutput, strlen(Glo.var.helpCallbackOutput));
        else if(strstr(helpCommand, "ticker")) UART_write(Glo.uart, Glo.var.helpTickerOutput, strlen(Glo.var.helpTickerOutput));
        else if(strstr(helpCommand, "reg")) UART_write(Glo.uart, Glo.var.helpRegOutput, strlen(Glo.var.helpRegOutput));
        else if(strstr(helpCommand, "script")) UART_write(Glo.uart, Glo.var.helpScriptOutput, strlen(Glo.var.helpScriptOutput));
        else if(strstr(helpCommand, "if")) UART_write(Glo.uart, Glo.var.helpIfOutput, strlen(Glo.var.helpIfOutput));
        else if(strstr(helpCommand, "uart")) UART_write(Glo.uart, Glo.var.helpUartOutput, strlen(Glo.var.helpUartOutput));
        else if(strstr(helpCommand, "sine")) UART_write(Glo.uart, Glo.var.helpSineOutput, strlen(Glo.var.helpSineOutput));
        else if(strstr(helpCommand, "netudp")) UART_write(Glo.uart, Glo.var.helpNetudpOutput, strlen(Glo.var.helpNetudpOutput));
        else if(strstr(helpCommand, "audio")) UART_write(Glo.uart, Glo.var.helpAudioOutput, strlen(Glo.var.helpAudioOutput));
        else{
            UART_write(Glo.uart, Glo.var.helpHelpOutput, indexOf(Glo.var.helpHelpOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpAboutOutput, indexOf(Glo.var.helpAboutOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpPrintOutput, indexOf(Glo.var.helpPrintOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpMemrOutput, indexOf(Glo.var.helpMemrOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpGPIOOutput, indexOf(Glo.var.helpGPIOOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpErrorOutput, indexOf(Glo.var.helpErrorOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpTimerOutput, indexOf(Glo.var.helpTimerOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpCallbackOutput, indexOf(Glo.var.helpCallbackOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpTickerOutput, indexOf(Glo.var.helpTickerOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpRegOutput, indexOf(Glo.var.helpRegOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpScriptOutput, indexOf(Glo.var.helpScriptOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpIfOutput, indexOf(Glo.var.helpIfOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpUartOutput, indexOf(Glo.var.helpUartOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpSineOutput, indexOf(Glo.var.helpSineOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpNetudpOutput, indexOf(Glo.var.helpNetudpOutput, '\t'));
            UART_write(Glo.uart, Glo.var.helpAudioOutput, indexOf(Glo.var.helpAudioOutput, '\t'));
       }
    }
    else if(commandTest(about, command)) UART_write(Glo.uart, Glo.var.aboutOutput, strlen(Glo.var.aboutOutput));
    else if(commandTest("-print", command)) {
        char *print;
        print = secondString(command);
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, print, strlen(print));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    }
    else if(commandTest("-memr", command)) {
        char *memoryBuffer;
        long memoryAddress = 0;
        long memoryContents = 0;
        char memoryString[MESSAGELEN] = "";
        char memoryAddressString[MESSAGELEN] = "";
        char *ptr;

        memoryBuffer = secondString(command);
        memoryAddress = strtol(memoryBuffer, &ptr, 16);
        if(memoryAddress < 0x00000000 || (memoryAddress > 0x000FFFFF && memoryAddress < 0x20000000) || (memoryAddress > 0x2003FFFF && memoryAddress < 0x40000000) || memoryAddress > 0x44055000){
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            UART_write(Glo.uart, "Error: Invalid Address\r\n", strlen("Error: Invalid Address\r\n"));
            errorCount[2]++;
        }

        else{
            memoryAddress = memoryAddress & 0xFFFFFFF0;
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            for(i = 0xC; i >= 0; i -= 0x4){
                sprintf(memoryAddressString, "0x%08lx\t\t", memoryAddress + i);
                UART_write(Glo.uart, &memoryAddressString, strlen(memoryAddressString));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            for(i = 0xC; i >= 0; i -= 0x4){
                memoryContents = *(int32_t *) (memoryAddress + i);
                sprintf(memoryString, "0x%08lx\t\t", memoryContents);
                UART_write(Glo.uart, memoryString, strlen(memoryString));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
    }
    else if(commandTest("-gpio", command)) {
        char *gpioBuffer;
        char *writeBuffer;
        char *pinBuffer;
        int pinNumber = 0;
        pinBuffer = secondString(command);
        pinNumber = atoi(pinBuffer);
        gpioBuffer = secondString(pinBuffer);
        if(commandTest("-w", gpioBuffer)){
            writeBuffer = secondString(gpioBuffer);
            if(commandTest("0", writeBuffer)) GPIO_write(pinNumber, 0);
            else if(commandTest("1", writeBuffer)) GPIO_write(pinNumber, 1);
            else GPIO_write(0, 1);
        }
        else if(commandTest("-r", gpioBuffer)){
//            UART_write(Glo.uart, "GPIO", 4);
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            if(GPIO_read(pinNumber)) UART_write(Glo.uart, "1", 1);
            else UART_write(Glo.uart, "0", 1);
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else if(commandTest("-t", gpioBuffer)){
            GPIO_toggle(pinNumber);

        }
        else{
            char buffer[20];
            for (i = 0; i < 8; i++){
                int state = GPIO_read(i) ? 1 : 0;
                sprintf(buffer, "GPIO %d: %d", i, state);
                UART_write(Glo.uart, buffer, strlen(buffer));
                UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
//                if(GPIO_read(i)) UART_write(Glo.uart, "1", 1);
//                else UART_write(Glo.uart, "0", 1);
//                UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
    }
    else if(commandTest("-error", command)) {
        char errorBufferOverflow[MESSAGELEN] = "";
        sprintf(errorBufferOverflow,         "Overflow:          %2d", errorCount[0]);
        char errorBufferIncorrectCommand[MESSAGELEN] = "";
        sprintf(errorBufferIncorrectCommand, "Incorrect Command: %2d", errorCount[1]);
        char errorBufferInvalidMemory[MESSAGELEN] = "";
        sprintf(errorBufferInvalidMemory,    "Invalid Memory:    %2d", errorCount[2]);
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, errorBufferOverflow, strlen(errorBufferOverflow));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, errorBufferIncorrectCommand, strlen(errorBufferIncorrectCommand));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, errorBufferInvalidMemory, strlen(errorBufferInvalidMemory));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    }
    else if(commandTest("-timer", command)) {
        char *periodBuffer;
        int period;
        periodBuffer = secondString(command);
        period = atoi(periodBuffer);
        if(period > 100) {
            Glo.callback[0].period = period;
            Timer_setPeriod(Glo.timer0, Timer_PERIOD_US, Glo.callback[0].period);
        }
        else if(!period){
            char currPerBuff[50];
            sprintf(currPerBuff, "Period is %d", Glo.callback[0].period);
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            UART_write(Glo.uart, currPerBuff, strlen(currPerBuff));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else{
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            UART_write(Glo.uart, "Invalid Period. Period must be > 100", strlen("Invalid Period. Period must be > 100"));
        }
    }
    else if(commandTest("-callback", command)) {
        char *callbackBuffer;
        callbackBuffer = secondString(command);
        char *numberBuffer;
        numberBuffer = secondString(callbackBuffer);
        char *payloadBuffer;
        payloadBuffer = secondString(numberBuffer);

        if(!callbackBuffer){
            UART_write(Glo.uart, "\r\nCallback 0 (Timer)       :\t", strlen("\r\nCallback 0 (Timer)       :\t"));
            UART_write(Glo.uart, Glo.callback[0].payload, strlen(Glo.callback[0].payload));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            UART_write(Glo.uart, "\r\nCallback 1 (Left Button) :\t", strlen("\r\nCallback 1 (Left Button) :\t"));
            UART_write(Glo.uart, Glo.callback[1].payload, strlen(Glo.callback[1].payload));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            UART_write(Glo.uart, "\r\nCallback 2 (Right Button):\t", strlen("\r\nCallback 2 (Right Button):\t"));
            UART_write(Glo.uart, Glo.callback[2].payload, strlen(Glo.callback[2].payload));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else if(!numberBuffer){
            if(atoi(callbackBuffer) == 0)
                UART_write(Glo.uart, "\r\nCallback 0 (Timer)       :\t", strlen("\r\nCallback 0 (Timer)       :\t"));
            else if(atoi(callbackBuffer) == 1)
                UART_write(Glo.uart, "\r\nCallback 1 (Left Button)       :\t", strlen("\r\nCallback 1 (Left Button)       :\t"));
            else if(atoi(callbackBuffer) == 2)
                UART_write(Glo.uart, "\r\nCallback 2 (Right Button)       :\t", strlen("\r\nCallback 2 (Right Button)       :\t")); // corrected here
            else
                UART_write(Glo.uart, "\r\nCallback Index is out of range       \t", strlen("\r\nCallback Index is out of range       \t"));
//                UART_write(Glo.uart, "\r\nCallback 1 (Left Button)       :\t", strlen("\r\nCallback 1 (Left Button)       :\t"));
//                UART_write(Glo.uart, "\r\nCallback 2 (Right Button)       :\t", strlen("\r\nCallback 2 (Right Button)       :\t")); // this seems redundant, by the way, due to the previous 'else if'

            UART_write(Glo.uart, Glo.callback[atoi(callbackBuffer)].payload, strlen(Glo.callback[atoi(callbackBuffer)].payload));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else{
            Glo.callback[atoi(callbackBuffer)].callbackCount = atoi(numberBuffer);
            strcpy(Glo.callback[atoi(callbackBuffer)].payload, payloadBuffer);
        }
    }
    else if(commandTest("-ticker", command)) {
        char *tickerIndexBuffer;
        tickerIndexBuffer = secondString(command);
        char *tickerDelayBuffer;
        tickerDelayBuffer = secondString(tickerIndexBuffer);
        char *tickerPeriodBuffer;
        tickerPeriodBuffer = secondString(tickerDelayBuffer);
        char *tickerCountBuffer;
        tickerCountBuffer = secondString(tickerPeriodBuffer);
        char *payloadBuffer;
        payloadBuffer = secondString(tickerCountBuffer);

        if(!tickerIndexBuffer || atoi(tickerIndexBuffer) < 0 || atoi(tickerIndexBuffer) >= TICKERNUM){
            int i = 0;
            for(i = 0; i < TICKERNUM; i++){
                char tickerOutputBuffer[48];
                sprintf(tickerOutputBuffer, "\r\nTicker %2d :\t", i);
                UART_write(Glo.uart, tickerOutputBuffer, strlen(tickerOutputBuffer));
                UART_write(Glo.uart, Glo.ticker[i].payload, strlen(Glo.ticker[i].payload));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else if(!tickerDelayBuffer){
            char tickerOutputBuffer[48];
            sprintf(tickerOutputBuffer, "\r\nTicker %2d :\t", atoi(tickerIndexBuffer));
            UART_write(Glo.uart, tickerOutputBuffer, strlen(tickerOutputBuffer));
            UART_write(Glo.uart, Glo.ticker[atoi(tickerIndexBuffer)].payload, strlen(Glo.ticker[atoi(tickerIndexBuffer)].payload));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else{
            Glo.ticker[atoi(tickerIndexBuffer)].delay = atoi(tickerDelayBuffer);

            Glo.ticker[atoi(tickerIndexBuffer)].period = atoi(tickerPeriodBuffer);

            Glo.ticker[atoi(tickerIndexBuffer)].tickerCount = atoi(tickerCountBuffer);

            strcpy(Glo.ticker[atoi(tickerIndexBuffer)].payload, payloadBuffer);
        }
    }
    else if(commandTest("-reg", command)) {
        char *regInBuffer;
        regInBuffer = secondString(command);
        char *regOperatorBuffer;
        regOperatorBuffer = secondString(regInBuffer);
        char *regOperandBuffer1;
        int32_t operand1;
        regOperandBuffer1 = secondString(regOperatorBuffer);
        if(commandTest("#", regOperandBuffer1)){
            regOperandBuffer1 = regOperandBuffer1 + 1;
            operand1 = atoi(regOperandBuffer1);
        }
        else if(atoi(regOperandBuffer1) < 0 || atoi(regOperandBuffer1) >= REGISTERNUM) operand1 = 0;
        else operand1 = Glo.reg[atoi(regOperandBuffer1)];

        char *regOperandBuffer2;
        int32_t operand2;
        regOperandBuffer2 = secondString(regOperandBuffer1);
        if(commandTest("#", regOperandBuffer2)){
            regOperandBuffer2 = regOperandBuffer2 + 1;
            operand2 = atoi(regOperandBuffer2);
        }
        else if(atoi(regOperandBuffer2) < 0 || atoi(regOperandBuffer2) >= REGISTERNUM) operand2 = 0;
        else operand2 = Glo.reg[atoi(regOperandBuffer2)];

        int i;

        if(!regInBuffer || atoi(regInBuffer) < 0 || atoi(regInBuffer) >= REGISTERNUM){
            for(i = 0; i < REGISTERNUM; i++){
                char regOutputBuffer[48];
                sprintf(regOutputBuffer, "\r\nRegister %2d :\t%4d", i, Glo.reg[i]);
                UART_write(Glo.uart, regOutputBuffer, strlen(regOutputBuffer));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            return;
        }
        else if(!regOperatorBuffer){
            char regOutputBuffer[48];
            sprintf(regOutputBuffer, "\r\nRegister %2d :\t%4d", atoi(regInBuffer), Glo.reg[atoi(regInBuffer)]);
            UART_write(Glo.uart, regOutputBuffer, strlen(regOutputBuffer));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
            return;
        }
        else if(commandTest("=", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1;
        else if(commandTest("++", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)] = operand1 + 1;
        else if(commandTest("--", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)] = operand1 - 1;
        else if(commandTest("+", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1 + operand2;
        else if(commandTest("-", regOperatorBuffer)){
            if(!regOperandBuffer2) Glo.reg[atoi(regInBuffer)] = -operand1;
            else Glo.reg[atoi(regInBuffer)] = operand1 - operand2;
        }
        else if(commandTest("~", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = ~operand1;
        else if(commandTest("&", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1 & operand2;
        else if(commandTest("|", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1 | operand2;
        else if(commandTest("^", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1 ^ operand2;
        else if(commandTest("*", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1 * operand2;
        else if(commandTest("/", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand2 != 0 ? operand1 / operand2 : 0x7FFFFFFF;
        else if(commandTest("%", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand2 != 0 ? operand1 % operand2 : 0x7FFFFFFF;
        else if(commandTest(">", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = (operand1 > operand2) ? operand1 : operand2;
        else if(commandTest("<", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = (operand1 < operand2) ? operand1 : operand2;
        else if(commandTest("c", regOperatorBuffer)) Glo.reg[atoi(regInBuffer)]  = operand1;
//        else if(commandTest("x", regOperatorBuffer)){
//            int32_t tempReg;
//            tempReg = Glo.reg[atoi(regInBuffer)];
//            Glo.reg[atoi(regInBuffer)]  = operand1;
//            if(operand1 >= 0 && operand1 < REGISTERNUM) Glo.reg[atoi(regOperandBuffer1)] = tempReg;
//            char regResultXBuffer[48];
//            sprintf(regResultXBuffer, "Register %d: %d", atoi(regOperandBuffer1), tempReg);
//            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
//            UART_write(Glo.uart, regResultXBuffer, strlen(regResultXBuffer));
//            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
//        }
        else if(commandTest("x", regOperatorBuffer)){
            // Convert the register indexes from string to integer
            int regIndex = atoi(regInBuffer);
            int operandIndex = atoi(regOperandBuffer1);

            // Check if the register indexes are within the allowed range
            if(regIndex < 0 || regIndex >= REGISTERNUM || operandIndex < 0 || operandIndex >= REGISTERNUM) {
                // Handle error for out-of-range index, you might want to send a message back through UART or handle it appropriately
                const char *errorMessage = "Error: Register index out of range.\r\n";
                UART_write(Glo.uart, errorMessage, strlen(errorMessage));
            } else {
                // Both indexes are in range, perform the exchange
                int32_t tempReg = Glo.reg[regIndex];  // Store the value of the first register temporarily
                Glo.reg[regIndex] = Glo.reg[operandIndex];  // Copy the value from the second register to the first
                Glo.reg[operandIndex] = tempReg;  // Complete the exchange by setting the second register to the temp value

                // Create a buffer to send the result/message back through UART
                char regResultXBuffer[48];
                sprintf(regResultXBuffer, "Register %d: %d\r\n",
                        operandIndex, Glo.reg[operandIndex]);
                UART_write(Glo.uart, regResultXBuffer, strlen(regResultXBuffer));
            }
        }
        char regResultBuffer[48];
        sprintf(regResultBuffer, "Register %d: %d", atoi(regInBuffer), Glo.reg[atoi(regInBuffer)]);
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, regResultBuffer, strlen(regResultBuffer));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    }
    else if(commandTest("-script", command)) {
        char *scriptSetBuffer;
        scriptSetBuffer = secondString(command);
        char *scriptIndexBuffer;
        scriptIndexBuffer = secondString(scriptSetBuffer);
        char *scriptCommandBuffer;
        scriptCommandBuffer = secondString(scriptIndexBuffer);

        int i;

        if((!scriptIndexBuffer && commandTest("-r",scriptSetBuffer)) || !scriptSetBuffer || atoi(scriptIndexBuffer) < 0 || atoi(scriptIndexBuffer) >= SCRIPTNUM){
            for(i = 0; i < SCRIPTNUM; i++){
                char scriptOutputBuffer[48];
                sprintf(scriptOutputBuffer, "\r\nScript %2d :\t%s", i, Glo.script[i].payload);
                UART_write(Glo.uart, scriptOutputBuffer, strlen(scriptOutputBuffer));
            }
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else if(commandTest("-r",scriptSetBuffer)){
            char scriptOutputBuffer[48];
            sprintf(scriptOutputBuffer, "\r\nScript %2d :\t%s", atoi(scriptIndexBuffer), Glo.script[atoi(scriptIndexBuffer)].payload);
            UART_write(Glo.uart, scriptOutputBuffer, strlen(scriptOutputBuffer));
            UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        }
        else if(!scriptIndexBuffer && commandTest("-x",scriptSetBuffer)){
            int j = 0;
            while(Glo.script[j].payload[0] != 0){
                addMessage(Glo.script[j].payload);
                Semaphore_post(Glo.msgQueSem);
                j++;
            }
        }
        else if(commandTest("-x",scriptSetBuffer)){
            int j = atoi(scriptIndexBuffer);
            while(Glo.script[j].payload[0] != 0){
                addMessage(Glo.script[j].payload);
                Semaphore_post(Glo.msgQueSem);
                j++;
            }
        }
        else if(commandTest("-w", scriptSetBuffer)){
            strcpy(Glo.script[atoi(scriptIndexBuffer)].payload, scriptCommandBuffer);
        }
    }
    else if(commandTest("-if", command)) {
        char *clause1Buffer;
        clause1Buffer = secondString(command);
        char *conditionalBuffer;
        conditionalBuffer = secondString(clause1Buffer);
        char *clause2Buffer;
        clause2Buffer = secondString(conditionalBuffer);
        char *resultTrueBuffer;
        resultTrueBuffer = secondString(clause2Buffer);
        resultTrueBuffer = secondString(resultTrueBuffer);
        char *colonBuffer;
        char *resultFalseBuffer;

        int i;
        i = indexOf(resultTrueBuffer, ':');
        colonBuffer=&resultTrueBuffer[i];
        if(colonBuffer[0] != ':') i++;
        resultTrueBuffer[i-1] = 0;
        resultFalseBuffer = secondString(colonBuffer);


        int32_t clause1;
        if(commandTest("#", clause1Buffer)){
            clause1Buffer = clause1Buffer + 1;
            clause1 = atoi(clause1Buffer);
        }
        else if(atoi(clause1Buffer) < 0 || atoi(clause1Buffer) >= REGISTERNUM) clause1 = 0;
        else clause1 = Glo.reg[atoi(clause1Buffer)];

        int32_t clause2;
        if(commandTest("#", clause2Buffer)){
            clause2Buffer = clause2Buffer + 1;
            clause2 = atoi(clause2Buffer);
        }
        else if(atoi(clause2Buffer) < 0 || atoi(clause2Buffer) >= REGISTERNUM) clause2 = 0;
        else clause2 = Glo.reg[atoi(clause2Buffer)];

        if(commandTest("<", conditionalBuffer)){
            if(clause1 < clause2){
                addMessage(resultTrueBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
            else {
                addMessage(resultFalseBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
        }
        else if(commandTest(">", conditionalBuffer)){
            if(clause1 > clause2){
                addMessage(resultTrueBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
            else {
                addMessage(resultFalseBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
        }
        else if(commandTest("==", conditionalBuffer)){
            if(clause1 == clause2){
                addMessage(resultTrueBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
            else{
                addMessage(resultFalseBuffer);
                Semaphore_post(Glo.msgQueSem);
            }
        }
    }
//    else if(commandTest("-uart", command)){
//        char *uartBuffer;
//        uartBuffer = secondString(command);
//        strcat(uartBuffer, "\n");
//        UART_write(Glo.uart7, uartBuffer, strlen(uartBuffer));
//    }
    else if(commandTest("-uart", command)){
        char *uartBuffer;
        uartBuffer = secondString(command);

        // Check if uartBuffer is NULL or an empty string (just containing the null terminator)
        if(uartBuffer == NULL || uartBuffer[0] == '\0'){
            // Handle the case of no additional command string
            const char *errorMessage = "Error: No command provided for -uart.\r\n";
            UART_write(Glo.uart, errorMessage, strlen(errorMessage));
        } else {
            // Prepare the string with an additional newline without modifying uartBuffer directly
            char sendBuffer[MESSAGELEN]; // Assuming MESSAGELEN is a predefined constant indicating max message length
            snprintf(sendBuffer, sizeof(sendBuffer), "%s\n", uartBuffer);
            UART_write(Glo.uart7, sendBuffer, strlen(sendBuffer));
        }
    }
    else if(commandTest("-sine", command)){
        char *sineBuffer;
        SPI_Transaction spiTransaction;
        sineBuffer = secondString(command);
        if(sineBuffer){
            if(atoi(sineBuffer) == 0){
                addMessage("-script -x 15");
                Semaphore_post(Glo.msgQueSem);
            }
            else{
                addMessage("-script -x 6");
                Semaphore_post(Glo.msgQueSem);
            }
            double sineFreq = atoi(sineBuffer);
            Glo.sine.indexJump = sineFreq*125.*256./1000000.;
        }
        Glo.sine.remainder = Glo.sine.sineIndex - floor(Glo.sine.sineIndex);
        Glo.sine.outVal = (int) Glo.sine.sineVal[(int) floor(Glo.sine.sineIndex)] * (1. - Glo.sine.remainder) + Glo.sine.sineVal[(int) floor(Glo.sine.sineIndex) + 1] * (Glo.sine.remainder);
        spiTransaction.count = 1;
        spiTransaction.txBuf = (void *) &Glo.sine.outVal;
        spiTransaction.rxBuf = (void *) NULL;
        SPI_transfer(Glo.sine.spi, &spiTransaction);
        Glo.sine.sineIndex += Glo.sine.indexJump;
        if(Glo.sine.sineIndex >= 256.) Glo.sine.sineIndex = Glo.sine.sineIndex - 256.;
    }
    else if(commandTest("-netudp", command)){
        // gate to protect netudp for only one at a time
        char *udpBuffer;
        udpBuffer = secondString(command);
        int i;
        i = indexOf(udpBuffer, ':');
        int j = 0;
        for(j = 0; j < i; j++) Glo.udp.ip[j] = udpBuffer[j];
        Glo.udp.ip[j] = 0;
        i++;
        for(j = 0; j < strlen(udpBuffer); j++){
            if(udpBuffer[j + i] == ' ') break;
            Glo.udp.port[j] = udpBuffer[j + i];
        }

        char *payloadBuffer;
        payloadBuffer = secondString(udpBuffer);

        if(commandTest("-mic", payloadBuffer)){
            if(!Glo.voice.voiceOut){
                addMessage("-gpio 7 -w 1");
                Semaphore_post(Glo.msgQueSem);

                ADCBuf_Conversion conversion;
                conversion.adcChannel = ADCBUF_CHANNEL_0;
                conversion.arg = NULL;
                conversion.sampleBuffer = Glo.adc.ping;
                conversion.sampleBufferTwo = Glo.adc.pong;
                conversion.samplesRequestedCount = VOICELEN;
                // Start ADCBuf conversion
                ADCBuf_convert(Glo.adc.bufferHandle, &conversion, 1);
            }
            else {
                addMessage("-gpio 7 -w 0");
                Semaphore_post(Glo.msgQueSem);
            }
            Glo.voice.voiceOut = !Glo.voice.voiceOut;
        }
        else{
            stringCopy(Glo.udp.payload, payloadBuffer);
            Semaphore_post(Glo.udp.sem);
        }
    }
    else if(commandTest("-audio", command)){
        if(!Glo.adc.audioOn){
            addMessage("-script -x 10");
            Semaphore_post(Glo.msgQueSem);
        }
        else {
            commandEntry("-callback 0 0");
            addMessage("-script -x 18");
            Semaphore_post(Glo.msgQueSem);
        }
        Glo.adc.audioOn = !Glo.adc.audioOn;
    }
    else if(commandTest("-stream", command)){
        if(!Glo.voice.voiceIn){
            addMessage("-script -x 11");
            Semaphore_post(Glo.msgQueSem);
        }
        else {
            commandEntry("-callback 0 0");
            addMessage("-script -x 19");
            Semaphore_post(Glo.msgQueSem);
        }
        Glo.voice.voiceIn = !Glo.voice.voiceIn;
    }
    else {
        errorCount[1]++;
        char errorMessage[48];
        sprintf(errorMessage, "%s Not Recognized as a Valid Command", command);
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
        UART_write(Glo.uart, errorMessage, strlen(errorMessage));
        UART_write(Glo.uart, Glo.var.newLine, strlen(Glo.var.newLine));
    }

}


int commandTest(const char *command, const char *compareString){
    int i;
    for (i = 0; i < strlen(command); i++){
        if(compareString[i] != command[i]) return 0;
    }
    return 1;
}

int indexOf(const char *string, const char character){
    int i;
    for(i = 0; i<strlen(string); i++) if(string[i] == character) break;
    return i;
}

void infra(void){
    Glo.var.helpAboutOutput = helpAboutOutput;
    Glo.var.input = input;
    Glo.var.echoPrompt = echoPrompt;
    Glo.var.helpHelpOutput = helpHelpOutput;
    Glo.var.helpPrintOutput = helpPrintOutput;
    Glo.var.helpMemrOutput = helpMemrOutput;
    Glo.var.helpGPIOOutput = helpGPIOOutput;
    Glo.var.helpErrorOutput = helpErrorOutput;
    Glo.var.helpTimerOutput = helpTimerOutput;
    Glo.var.helpCallbackOutput = helpCallbackOutput;
    Glo.var.helpTickerOutput = helpTickerOutput;
    Glo.var.helpRegOutput = helpRegOutput;
    Glo.var.helpScriptOutput = helpScriptOutput;
    Glo.var.helpIfOutput = helpIfOutput;
    Glo.var.helpUartOutput = helpUartOutput;
    Glo.var.helpSineOutput = helpSineOutput;
    Glo.var.helpNetudpOutput = helpNetudpOutput;
    Glo.var.helpAudioOutput = helpAudioOutput;

    Glo.var.help = help;
    Glo.var.about = about;
    Glo.var.command = command;
    Glo.var.previousCommand = previousCommand;
    Glo.var.callbackCommand = callbackCommand;

    Glo.var.i = i;
    Glo.var.arrowInput = arrowInput;
    Glo.var.errorCount[0] = errorCount[0];
    Glo.var.errorCount[1] = errorCount[1];
    Glo.var.errorCount[2] = errorCount[2];
    Glo.var.overflowFlag = overflowFlag;

    sprintf(aboutOutput, "\r\nRishikesh, Assignment 10, v12.4.0.00007, %s, %s\r\n", __TIME__, __DATE__); //put outside subroutine
    Glo.var.aboutOutput = aboutOutput;
    Glo.callback[0].callbackCount = timerCallbackCount;
    Glo.callback[1].callbackCount = leftButtonCallbackCount;
    Glo.callback[2].callbackCount = rightButtonCallbackCount;
    Glo.var.messageIndex = messageIndex;
    Glo.var.newLine = newLine;
    Glo.msgQue.readIndex = 0;
    Glo.msgQue.writeIndex = 0;
    Glo.msgQueSem = semaphore1;
    Glo.callback[0].payload[0] = 0;
    Glo.callback[1].payload[0] = 0;
    Glo.callback[2].payload[0] = 0;
    Glo.callback[0].period = callbackPeriod;

    for(i = 0; i < TICKERNUM; i++){
        Glo.ticker[i].period = 0;
        Glo.ticker[i].delay = 0;
        Glo.ticker[i].tickerCount = 0;
        Glo.ticker[i].payload[0] = 0;
    }

    for(i = 0; i < REGISTERNUM; i++){
        Glo.reg[i] = i * 10;
    }

    for(i = 0; i < SCRIPTNUM; i++){
        Glo.script[i].payload[0] = 0;
    }
    // test conditional script, if true it jumps back to 0 and then picks up where it left off when done
    stringCopy(Glo.script[0].payload, "-gpio 0 -t");
    stringCopy(Glo.script[2].payload, "-if 4 > 3 ? -script -x 0");
    stringCopy(Glo.script[3].payload, "-gpio 1 -t");

    // sine on
    stringCopy(Glo.script[6].payload, "-gpio 6 -w 0");
    stringCopy(Glo.script[7].payload, "-timer 125");
    stringCopy(Glo.script[8].payload, "-callback 0 -1 -sine");

    // mic on
    stringCopy(Glo.script[10].payload, "-gpio 7 -w 1");
    stringCopy(Glo.script[11].payload, "-gpio 6 -w 0");
    stringCopy(Glo.script[12].payload, "-timer 125");
    stringCopy(Glo.script[13].payload, "-callback 0 -1");



    // sine off
    stringCopy(Glo.script[15].payload, "-gpio 6 -w 1");
    stringCopy(Glo.script[16].payload, "-callback 0 0");

    // mic off
    stringCopy(Glo.script[18].payload, "-gpio 7 -w 0");
    stringCopy(Glo.script[19].payload, "-gpio 6 -w 1");
    stringCopy(Glo.script[20].payload, "-callback 0 0");



    // LED Dancing
    stringCopy(Glo.script[22].payload, "-ticker 0 100 100 1 -gpio 0 -t");
    stringCopy(Glo.script[23].payload, "-ticker 1 200 100 1 -gpio 1 -t");
    stringCopy(Glo.script[24].payload, "-ticker 2 300 100 1 -gpio 2 -t");
    stringCopy(Glo.script[25].payload, "-ticker 3 400 100 1 -gpio 3 -t");
    stringCopy(Glo.script[26].payload, "-ticker 4 500 100 1 -gpio 3 -t");
    stringCopy(Glo.script[27].payload, "-ticker 5 600 100 1 -gpio 2 -t");
    stringCopy(Glo.script[28].payload, "-ticker 6 700 100 1 -gpio 1 -t");
    stringCopy(Glo.script[29].payload, "-ticker 7 800 100 1 -gpio 0 -t");

    // IF statements
    stringCopy(Glo.script[41].payload, "-if 4 > #3 ? -gpio 0 -t : -gpio 1 -t");
    stringCopy(Glo.script[47].payload, "-if 10 > #0 ? -script -x 0 : -script -x 3");



    Glo.sine.sineVal = sineVal;
    Glo.sine.sineIndex = sineIndex;
    Glo.sine.indexJump = indexJump;
    Glo.sine.remainder = sineRemainder;
    Glo.sine.outVal = outVal;

    Glo.udp.sem = semaphore0;

    Glo.adc.audioOn = false;

    Glo.voice.voiceIn = 0;
    Glo.voice.voiceOut = 0;
    Glo.voice.index = 0;

    Glo.adc.pp = 0;
}

void initializeDrivers(void){
    Timer_Handle timer0, timer1;
    Timer_Params params;
    UART_Handle uart, uart7;
    UART_Params uartParams;
    SPI_Handle spi;
    SPI_Params spiParams;
    ADC_Handle adcHandle;
    ADC_Params adcParams;
    ADCBuf_Handle adcBufHandle;
    ADCBuf_Params adcBufParams;


    /* Call driver init functions */
    Board_init();
    Timer_init();
    UART_init();
    GPIO_init();
    GPIO_enableInt(4);
    GPIO_enableInt(5);
    SPI_init();
    ADC_init();
    ADC_Params_init(&adcParams);
    ADCBuf_init();
    ADCBuf_Params_init(&adcBufParams);

    Timer_Params_init(&params);
    params.period = Glo.callback[0].period;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;

    timer0 = Timer_open(CONFIG_TIMER_0, &params);

    if (timer0 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
    /* Call driver init functions */
    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }

    Timer_Params_init(&params);
    params.period = 10000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = tickerCallback;

    timer1 = Timer_open(CONFIG_TIMER_1, &params);

    if (timer1 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
    /* Call driver init functions */
    if (Timer_start(timer1) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;

    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart7 = UART_open(CONFIG_UART_7, &uartParams);

    if (uart7 == NULL) {
        /* UART_open() failed */
        while (1);
    }

    Glo.timer0 = timer0;
    Glo.timer1 = timer1;
    Glo.uart = uart;
    Glo.uart7 = uart7;

    SPI_Params_init(&spiParams);
    spiParams.dataSize = 16;

    spiParams.frameFormat = SPI_POL0_PHA1;
    spi = SPI_open(CONFIG_SPI_0, &spiParams);
    //dac datasheet
    if (spi == NULL){
        while(1);
    }

    Glo.sine.spi = spi;

    adcParams.isProtected = true;

    adcHandle = ADC_open(0, &adcParams);
    if (adcHandle == NULL)
    {
        //ADCBuf_open() failed.
        while (1) {}
    }
    Glo.adc.handle = adcHandle;

    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_CONTINUOUS;
    adcBufParams.callbackFxn = adcCallback;
    adcBufParams.samplingFrequency = 8000;

    adcBufHandle = ADCBuf_open(CONFIG_ADCBUF_0, &adcBufParams);
    Glo.adc.bufferHandle = adcBufHandle;
}

void leftButtonCallback(void){
    if(Glo.callback[1].callbackCount != 0){
        addMessage(Glo.callback[1].payload);
        Semaphore_post(Glo.msgQueSem);
        if(Glo.callback[1].callbackCount>0){
            Glo.callback[1].callbackCount--;
        }
    }
}

void rightButtonCallback(void){
    if(Glo.callback[2].callbackCount != 0){
        addMessage(Glo.callback[2].payload);
        Semaphore_post(Glo.msgQueSem);
        if(Glo.callback[2].callbackCount>0){
            Glo.callback[2].callbackCount--;
        }
    }
}

char *secondString(char *fullString){
    char *outString = NULL;
    int i = 0;
    for (i = 0; i < strlen(fullString); i++){
        if(fullString[i] == ' '){
            outString = &fullString[i + 1];
            return outString;
        }
    }
    return outString;
}

void stringCopy(char *outString, const char *copiedString){
    for(i = 0; i < MESSAGELEN; i++) if(copiedString[i] == 0) break;
    int j = 0;
    for(j = 0; j < i; j++){
        outString[j] = copiedString[j];
    }
    outString[j] = 0;
}

void taskCommandServicer(void){
    while(1){
        Semaphore_pend(Glo.msgQueSem, BIOS_WAIT_FOREVER);

        commandEntry(Glo.msgQue.messages[Glo.msgQue.readIndex].message);
        Glo.msgQue.readIndex++;
        if(Glo.msgQue.readIndex == QUELEN) Glo.msgQue.readIndex = 0;
    }
}

void tickerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    int i;
    for(i = 0; i < TICKERNUM; i++){
        if(Glo.ticker[i].tickerCount != 0 && Glo.ticker[i].delay == 0){
            addMessage(Glo.ticker[i].payload);
            Semaphore_post(Glo.msgQueSem);
            if(Glo.ticker[i].tickerCount>0){
                Glo.ticker[i].tickerCount--;
            }
            Glo.ticker[i].delay = Glo.ticker[i].period;
        }
        else if(Glo.ticker[i].delay != 0){
            Glo.ticker[i].delay--;
        }
    }
}

void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    if(Glo.callback[0].callbackCount != 0){
        if(Glo.adc.audioOn) {
            uint16_t convBuffer;
            ADC_convert(Glo.adc.handle, &convBuffer);
            convBuffer *= 2;
            audioFoo(convBuffer);
        }
        else if(Glo.voice.voiceIn) {
            if(!Glo.voice.pp) audioFoo(Glo.voice.ping[Glo.voice.index++]);
            else audioFoo(Glo.voice.pong[Glo.voice.index++]);
            if(Glo.voice.index >= VOICELEN){
                Glo.voice.index = 0;
            }
        }
        else if(commandTest("-sine", Glo.callback[0].payload)) commandEntry(Glo.callback[0].payload);
        else{
            addMessage(Glo.callback[0].payload);
            Semaphore_post(Glo.msgQueSem);
        }
        if(Glo.callback[0].callbackCount>0){
            Glo.callback[0].callbackCount--;
        }
    }
}

void adcCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *buffer, uint32_t channel, int_fast16_t status){
    char printBuff[MESSAGELEN];

    if(Glo.adc.pp == 0){
        sprintf(printBuff, "-voice 0 128  ");
        stringCopy(Glo.udp.payload, printBuff);
        Glo.udp.payload[strlen(printBuff)] = 0;
        memcpy(&Glo.udp.payload[strlen(printBuff) + 1], Glo.adc.ping, sizeof(uint16_t) * VOICELEN);
    }
    else{
        sprintf(printBuff, "-voice 1 128  ");
        stringCopy(Glo.udp.payload, printBuff);
        Glo.udp.payload[strlen(printBuff)] = 0;
        memcpy(&Glo.udp.payload[strlen(printBuff) + 1], Glo.adc.pong, sizeof(uint16_t) * VOICELEN);
    }

    Glo.adc.pp = !Glo.adc.pp;

    if(Glo.voice.voiceOut) {
        ADCBuf_Conversion conversion = {0};
        conversion.samplesRequestedCount = VOICELEN;
        if(Glo.adc.pp == 0) conversion.sampleBuffer = Glo.adc.ping;
        else conversion.sampleBuffer = Glo.adc.pong;
        conversion.adcChannel = 0;
        // Start ADCBuf conversion
        ADCBuf_convert(Glo.adc.bufferHandle, &conversion, 1);
    }
    Semaphore_post(Glo.udp.sem);
}




