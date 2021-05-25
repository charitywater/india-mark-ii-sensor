/**************************************************************************************************
* \file     taskMonitor.h
* \brief    Tasks 'check in' regularly when running properly. If a task fails to check in after X ms,
*           the task monitor will not refresh the watchdog, resulting in a system reset.
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
#ifndef HANDLERS_TASKMONITOR_H_
#define HANDLERS_TASKMONITOR_H_


extern void TM_initialize(void);
extern void TM_task();

/* check in functions for tasks...add one function
   for each task */
extern void TM_cliTaskCheckIn(void);
extern void TM_mainTaskCheckIn(void);
extern void TM_ssmTaskCheckIn(void);
extern void TM_connTaskCheckIn(void);

#endif /* HANDLERS_TASKMONITOR_H_ */
