/**************************************************************************************************
* \file     CLI.h
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

#ifndef HANDLERS_CLI_H_
#define HANDLERS_CLI_H_

#include <stdint.h>

#define COMMAND_IDX     0
#define FIRST_ARG_IDX   1
#define SECOND_ARG_IDX  2
#define THIRD_ARG_IDX   3
#define FOURTH_ARG_IDX  4

//structure is [command][argument 1][argument 2]...
//so if there is 1 argument, there will be two fields
#define ONE_ARGUMENT    2
#define TWO_ARGUMENTS   3
#define THREE_ARGUMENTS 4
#define FOUR_ARGUMENTS  5

// ---------------------------------------------------------------------+-
// Defines the signature of a command handler function.
// ---------------------------------------------------------------------+-
typedef void (CLI_Command_Handler_Function_t)(int argc, char **argv);


// ---------------------------------------------------------------------+-
// Describes a command line handler.
// cmdString is the unique string which the dispatcher will try
// to match against the first argument on the incoming command line.
// ---------------------------------------------------------------------+-
typedef struct {
    CLI_Command_Handler_Function_t *ptrFunction;
    const char *cmdString;
    const char *usageString;
} CLI_Command_Handler_s;

extern void CLI_init(void);
extern void CLI_commandLineHandler_task();
extern void CLI_registerThisCommandHandler(CLI_Command_Handler_s *ptrStruct);
extern void CLI_print(char* msg, ...);
extern void CLI_printNoNewline(char* msg, ...);
extern void CLI_charReceivedCb(void);

#endif /* HANDLERS_CLI_H_ */
