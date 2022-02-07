/**************************************************************************************************
* \file     taskMonitor.c
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


/* Includes */
#include "stddef.h"
#include "logTypes.h"
#include "watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "taskMonitor.h"

//due to some tasks blocking on various thing, we
//will not make each task check in at the same rate we kick the watchdog
//(also the watchdog has a max kick of 2ms at our clock rate...)
#define TASK_WATCHDOG_KICK_RATE_MS       20 * 1000

/* Incoming message types
One ENUM for each task...ADD MORE later */
typedef enum
{
    CLI_TASK_IS_ALIVE_MSG,
    CONN_TASK_IS_ALIVE_MSG,
    MAIN_TASK_IS_ALIVE_MSG,
    MAX_TASK_MONITOR_MSG
} TaskMonitor_MessageID_e;

typedef enum
{
    UNKNOWN,
    ALIVE
} TaskStatus_e;

const char* TaskNames[] = { "CLI",
                            "CONN"
                            "MAIN"
                            "MAX TASKS MSG" };

/* flags to indicate if tasks are alive or not */
TaskStatus_e taskFlags[MAX_TASK_MONITOR_MSG] = { UNKNOWN };

static uint32_t checkInCounterMs = 0u;
static uint32_t wdCheckInRateMs = 0u;

void TM_initialize(void)
{
    /* check if we have just recovered from a WD reset */
    if ( WD_recoveredFromReset() == true )
    {
        //do we want to do anything? log to the cloud? etc
    }

    /* init the wd peripheral */
    WD_init();

    wdCheckInRateMs = WD_getRefreshRateMs();

}

void TM_task()
{
    while(1)
    {
        /* delay until its time to refresh the WD */
        vTaskDelay(wdCheckInRateMs);

        checkInCounterMs += wdCheckInRateMs;

        //if we havent reached the task check in rate yet, just kick the wd
        if ( checkInCounterMs < TASK_WATCHDOG_KICK_RATE_MS )
        {
            // kick the watchdog
            WD_refresh();
        }
        else
        {
            //reset counter
            checkInCounterMs = 0;

            //check if we need to reset the watchdog now
            bool allTasksCheckedIn = true;

            /* make sure all tasks checked in */
            for (uint8_t i = 0; i < MAX_TASK_MONITOR_MSG; i++)
            {
                if (taskFlags[i] == UNKNOWN)
                {
                    elogError("Thread %s did not check in!", TaskNames[i]);
                    allTasksCheckedIn = false;
                }

                /* reset value */
                taskFlags[i] = UNKNOWN;
            }

            if (allTasksCheckedIn == true)
            {
                /* refresh watchdog */
                WD_refresh();
            }
        }
    }
}


//ADD check in messages here as tasks are added
void TM_cliTaskCheckIn(void)
{
    /* update flag to indicate that this task is alive */
    taskFlags[CLI_TASK_IS_ALIVE_MSG] = ALIVE;
}

void TM_ssmTaskCheckIn(void)
{
    /* update flag to indicate that this task is alive */
    //taskFlags[SSM_TASK_IS_ALIVE_MSG] = ALIVE;
}

void TM_connTaskCheckIn(void)
{
    /* update flag to indicate that this task is alive */
    taskFlags[CONN_TASK_IS_ALIVE_MSG] = ALIVE;
}

void TM_mainTaskCheckIn(void)
{
    /* update flag to indicate that this task is alive */
    taskFlags[MAIN_TASK_IS_ALIVE_MSG] = ALIVE;
}
