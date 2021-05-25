/**************************************************************************************************
* \file     CLI.c
* \brief    API to register command handlers and to process user input received via the command line
*
* \par      Copyright Notice
*           Copyright 2021 charity: water
*
*           Licensed under the Apache License, Version 2.0 (the "License");
*           you may not use this file except in compliance with the License.
*           You may obtain a copy of the License at
*
*               http://www.apache.org/licenses/LICENSE-2.0
*
*           Unless required by applicable law or agreed to in writing, software
*           distributed under the License is distributed on an "AS IS" BASIS,
*           WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*           See the License for the specific language governing permissions and
*           limitations under the License.
*           
* \date     2/1/2021
* \author   Twisthink
*
***************************************************************************************************/
#include <stm32l4xx_hal.h>
#include "taskMonitor.h"
#include <uart.h>
#include "stdint.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logTypes.h"
#include <CLI.h>

#define MAX_USER_INPUT_BYTES            50
#define QUEUE_TIMEOUT_IN_MS             1000
#define MAX_REGISTERED_HANDLERS         24
#define DEFAULT_HANDLER_IDX             (MAX_REGISTERED_HANDLERS - 1)
#define MAXARGS                         12
#define CLI_PRINT_MAX_LINE              250
#define CHAR_WAIT_TIMEOUT_MS            1000
#define CLI_TASK_POLL_RATE_MS           30
#define CLI_MSG_LEN                     9

static CLI_Command_Handler_s  Handler_List[MAX_REGISTERED_HANDLERS] = {{0}};
uint8_t userInputBuffer[MAX_USER_INPUT_BYTES];
static uint8_t commandBuffer[MAX_USER_INPUT_BYTES];
static uint8_t commandBufferIdx = 0;
uint8_t userInputBufferHeadIdx = 0u;
uint8_t userInputBufferTailIdx = 0u;
static char charLineBuf[CLI_PRINT_MAX_LINE];
static char cliMsg[CLI_MSG_LEN] = "\r\nAM CLI>";
uint8_t byteReceived = 0;

//private functions
static CLI_Command_Handler_Function_t *xFindHandler(char *givenCommand);
static void xPrintHelp(void);
static void xPrintCLIMsg(void);
static void xInvokeTheHandlerForThisCommand(char *givenBuff, uint32_t maxLen);
static void xRegisterThisDefaultCommandHandler(CLI_Command_Handler_s *ptrGivenHandler);
static void xDefaultCommandHandler(int argc, char **argv);
static void xCliTestCommandHandler(int argc, char **argv);

//Interrupt callback function for received characters. Add character to
//buffer and kick off another receive. The buffer is processed in the task
//starting at the tail index.
void CLI_charReceivedCb(void)
{
    //buffer up received data for the task to parse
    userInputBuffer[userInputBufferHeadIdx] = byteReceived;

    userInputBufferHeadIdx++;

    if(userInputBufferHeadIdx >= MAX_USER_INPUT_BYTES )
    {
        userInputBufferHeadIdx = 0;
    }

    //receive another byte
    UART_recieveDataNonBlocking(CLI, &byteReceived, 1);
}


// ---------------------------------------------------------------------+-
// Task for handing user inputs. Kicks off a 'receive' by providing a
// buffer for the uart receive interrupt to place data
// ---------------------------------------------------------------------+-
void CLI_commandLineHandler_task()
{
    CLI_init();

    while(1)
    {
        //if we have new data in the buffer, process it
        if ( userInputBufferHeadIdx != userInputBufferTailIdx)
        {
            while (userInputBufferHeadIdx != userInputBufferTailIdx )
            {
                //build a command
                commandBuffer[commandBufferIdx] = userInputBuffer[userInputBufferTailIdx];

                userInputBufferTailIdx++;

                //make sure there hasnt been an overflow
                if(userInputBufferTailIdx >= MAX_USER_INPUT_BYTES)
                {
                    userInputBufferTailIdx = 0u;
                }

                //echo the char
                UART_sendDataBlocking(CLI, &commandBuffer[commandBufferIdx], 1);

                //check for return character
                if(strcmp((char*)&commandBuffer[commandBufferIdx],"\r") == 0)
                {
                    //Remove the last character
                    commandBuffer[commandBufferIdx] = 0u;

                    //Call the driver with our buffer of data for it to handle
                    xInvokeTheHandlerForThisCommand((char *)commandBuffer, MAX_USER_INPUT_BYTES);

                    //reset buffer and index
                    memset(&commandBuffer, 0, MAX_USER_INPUT_BYTES);

                    commandBufferIdx = 0u;

                    xPrintCLIMsg();
                }
                else
                {
                    commandBufferIdx++;

                    if (commandBufferIdx >= MAX_USER_INPUT_BYTES)
                    {
                        commandBufferIdx = 0u;
                    }
                }
            }
        }

        TM_cliTaskCheckIn();

        vTaskDelay(CLI_TASK_POLL_RATE_MS);
    }
}


// ---------------------------------------------------------------------+-
// Initialize the command line handler
// ---------------------------------------------------------------------+-
void CLI_init(void)
{
    // Define default command handler.
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &xDefaultCommandHandler;
    cmdHandler.cmdString   = "dflt";
    cmdHandler.usageString = "default command handler";

    CLI_Command_Handler_s cliTestCmdHandler;
    cliTestCmdHandler.ptrFunction = &xCliTestCommandHandler;
    cliTestCmdHandler.cmdString   = "cli";
    cliTestCmdHandler.usageString = "\n\r\ttest";

    CLI_registerThisCommandHandler(&cliTestCmdHandler);
    xRegisterThisDefaultCommandHandler(&cmdHandler);

    //init the uart peripheral
    UART_initCliUart();

    xPrintCLIMsg();

    //enable interrupts, clear any pending ones
    HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    //kick off receive
    UART_recieveDataNonBlocking(CLI, &byteReceived, 1);

    elogInfo("CLI Initialized");
}

void CLI_registerThisCommandHandler(CLI_Command_Handler_s *ptrGivenHandler)
{
    uint8_t idx;

    // Find the first empty slot;
    // but don't use the last one, that's for the default handler.
    for (idx = 0; idx < DEFAULT_HANDLER_IDX; idx++)
    {
        // Copy the given struct into the empty slot.
        if( Handler_List[idx].ptrFunction == (CLI_Command_Handler_Function_t *)NULL)
        {
            Handler_List[idx] = *ptrGivenHandler;
            break;
        }
    }
    if(idx >= DEFAULT_HANDLER_IDX)
    {
        CLI_print("\r\nNo room for %s cmd", ptrGivenHandler->cmdString);
    }
};

void CLI_printNoNewline(char* msg, ...)
{
    va_list argp;

    /** - Format the string */
    va_start(argp, msg );
    vsnprintf(&charLineBuf[0], sizeof(charLineBuf), msg, argp);
    va_end(argp);

    /** - print out the message */
    UART_sendDataBlocking(CLI, (uint8_t*)charLineBuf,  strlen(charLineBuf));
}

void CLI_print(char* msg, ...)
{
    va_list argp;

    /** - Format the string */
    va_start(argp, msg );
    vsnprintf(&charLineBuf[0], sizeof(charLineBuf), msg, argp);
    va_end(argp);
    strcat(charLineBuf, "\r\n");

    /** - print out the message */
    UART_sendDataBlocking(CLI, (uint8_t*)charLineBuf,  strlen(charLineBuf));
}

static CLI_Command_Handler_Function_t *xFindHandler(char *givenCommand)
{
    CLI_Command_Handler_Function_t *foundHandler = (CLI_Command_Handler_Function_t *)NULL;

    for (int idx = 0; idx < DEFAULT_HANDLER_IDX; idx++)
    {
        if ((CLI_Command_Handler_Function_t *)NULL == Handler_List[idx].ptrFunction)
        {
            continue;
        }
        if (0 == strcmp(givenCommand, Handler_List[idx].cmdString))
        {
            foundHandler = Handler_List[idx].ptrFunction;
            break;
        }
    }
    return foundHandler;
};


static void xPrintHelp(void)
{
    CLI_print("\r\n");
    CLI_print("AM CLI COMMANDS: ");
    CLI_print("\r\n");

    for (int idx = 0; idx < DEFAULT_HANDLER_IDX; idx++) {

        if ((CLI_Command_Handler_Function_t *)NULL == Handler_List[idx].ptrFunction)
        {
            continue;
        }

        // Print usage for this command.
        CLI_print(
            "%s - %s ",
            Handler_List[idx].cmdString,
            Handler_List[idx].usageString );
    }
};

static void xPrintCLIMsg(void)
{
    UART_sendDataBlocking(CLI, (uint8_t*)cliMsg, sizeof(cliMsg));
}


// ---------------------------------------------------------------------+-
// Given a single character,
// return true iff this character is not valid
// as part of an argument, i.e., look for something
// that would separate one arg from another,
// typically some whitespace character.
// ---------------------------------------------------------------------+-
static inline bool isNotValidArgChar(char c)
{
    if (c == '\x20') return true;  // space
    if (c == '\t')   return true;  // tab
    if (c == '\n')   return true;  // newline
    if (c == '\0')   return true;  // null
    return false;
};

static inline bool isNull(char c)
{
    if (c == '\0')   return true;
    return false;
};

static inline bool isValidArgChar(char c)
{
    return !isNotValidArgChar(c);
};


static void xInvokeTheHandlerForThisCommand(char *givenBuff, uint32_t maxLen)
{
    //elogDebug( "givenBuff: |%s|", givenBuff);

    bool endOfCommand = false;
    int  pos = 0;

    char *argv[MAXARGS];
    int   argc = 0;

    // ---------------------------------------------------------------------+-
    // Parse given command line buff into argc and argv
    // ---------------------------------------------------------------------+-
    while(true) {

        // Consume any leading whitespace to find start of next arg
        while(isNotValidArgChar(givenBuff[pos])) {

            if(isNull(givenBuff[pos])) {
                endOfCommand = true;
                break;
            }
            pos++;
            if (pos >= maxLen) {
                endOfCommand = true;
                break;
            }
        }
        if (endOfCommand) break;

        // Save a pointer to first char of next arg and increment arg count.
        argv[argc++] = &givenBuff[pos];

        // Advance to first separator after this new argument.
        while(isValidArgChar(givenBuff[pos])) {
            pos++;
            if (pos >= maxLen) {
                endOfCommand = true;
                break;
            }
            if  ( true == isNull(givenBuff[pos]) )
            {
                endOfCommand = true;
                break;
            }
        }
        if (endOfCommand) break;

        // Replace that separator with NULL term char.
        givenBuff[pos] = '\0';

        // Advance to the next char.
        // And parse the next char if we are not at the end of the buffer.
        pos++;
        if (pos >= maxLen) {
            endOfCommand = true;
            break;
        }
    }

    // Nothing to invoke, when command is empty.
    if (argc == 0) return;

    // ---------------------------------------------------------------------+-
    // Invoke the corresponding handler if one can be matched.
    // ---------------------------------------------------------------------+-
    CLI_Command_Handler_Function_t *handler = xFindHandler(argv[0]);
    if (handler != NULL) {
        handler(argc, argv);
        return;
    }

    // ---------------------------------------------------------------------+-
    // Otherwise, print help, as needed.
    // ---------------------------------------------------------------------+-
    if (0 == strcmp("help", argv[0])) {
        xPrintHelp();
        return;
    }

    // ---------------------------------------------------------------------+-
    // Otherwise, invoke the default handler, if there is one.
    // ---------------------------------------------------------------------+-
    if (Handler_List[DEFAULT_HANDLER_IDX].ptrFunction != NULL) {
        handler = Handler_List[DEFAULT_HANDLER_IDX].ptrFunction;
        handler(argc, argv);
        return;
    }

    CLI_print("\r\nno default handler");
};

static void xRegisterThisDefaultCommandHandler(CLI_Command_Handler_s *ptrGivenHandler)
{
    Handler_List[DEFAULT_HANDLER_IDX] = *ptrGivenHandler;
};

// ---------------------------------------------------------------------+-
// Default command handler for when the user has entered an invalid command
// ---------------------------------------------------------------------+-
static void xDefaultCommandHandler(int argc, char **argv)
{
    if (argc < 1) {
        CLI_print("\r\nUnknown command.");
        return;
    }
    if (argc >= 1) {
        CLI_print("\r\nUnknown command: %s", argv[0]);
        return;
    }
};

static void xCliTestCommandHandler(int argc, char **argv)
{
    if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "test") )
    {
        CLI_print("\r\nTEST CLI CMD");
    }
    else
    {
        CLI_print("\r\nInvalid arg");
    }
}
