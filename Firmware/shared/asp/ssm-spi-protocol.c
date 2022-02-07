/**************************************************************************************************
* \file     ssm-spi-protocol.c
* \brief    Protocol functionality specific to the SSM.
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
#ifdef SSM_BUILD

#include "am-ssm-spi-protocol.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "HW_RTC.h"
#include "APP_ALGO.h"
#include "version.h"
#include "HW.h"
#include "HW_BAT.h"
#include "APP.h"
#include "APP_NVM.h"

void ASP_SSM_Periodic(void);
void ASP_HandleCommandMsg(asp_msg_t * p_msg);
void ASP_HandleGetSensorDataMsg(void);
void ASP_TransmitStatus(void);
void ASP_TransmitBytesInSensorDataLog(void);
void ASP_TransmitSensorDataLog(APP_NVM_SENSOR_DATA_T * p_sensorData);
void ASP_HandleSetRTCMsg(asp_msg_t * p_msg);
void ASP_HandleAttnSourceAckMsg(asp_attn_source_payload_t *pMsg);
void ASP_HandleConfigMsg(asp_msg_t *pMsg);
void ASP_TransmitAck(uint8_t ackId);
void ASP_HandleErroneousMsg(void);

// Periodic function for ASP coms.
void ASP_SSM_Periodic(void)
{
	while ( uC_SPI_BytesReady() )
		ASP_ProcessIncomingByte( uC_SPI_GetNextByte() );
}

// A get log message was received.  Identify the command and respond.
void ASP_HandleGetSensorDataMsg(void)
{
    APP_NVM_SENSOR_DATA_T sensorData = {};

    if ( APP_NVM_GetSensorData(&sensorData) )
    {
        //send the spi message
        ASP_TransmitSensorDataLog(&sensorData);
    }
    else
    {
        ASP_HandleErroneousMsg();
    }
}

// A get log message was received.  Identify the command and respond.
void ASP_HandleSetRTCMsg(asp_msg_t * p_msg)
{
    uint32_t time = 0x00000000;

    //first Ack the msg
    ASP_TransmitAck((uint8_t)ASP_SET_RTC_MSG_ID);

    time = (p_msg->fields.payload.setRTC.RTC_time);

    if (HW_RTC_SetTimeEpoch(time))
    {
        APP_setTimeUpdated();
    }
    else
    {
        APP_setTimeFailed();
    }
}

//new config msg received
void ASP_HandleConfigMsg(asp_msg_t *pMsg)
{
    //first Ack the msg if it is valid
    if (pMsg->fields.payload.configParams.wakeIntervalInDays <= MAX_WAKE_AM_RATE_DAYS &&
            pMsg->fields.payload.configParams.wakeIntervalInDays >= MIN_WAKE_AM_RATE_DAYS &&
            pMsg->fields.payload.configParams.redFlagOnThreshold >= MIN_RED_FLAG_ON_THRESH &&
            pMsg->fields.payload.configParams.redFlagOnThreshold <= MAX_RED_FLAG_ON_THRESH &&
            pMsg->fields.payload.configParams.redFlagOffThreshold >= MIN_RED_FLAG_OFF_THRESH &&
            pMsg->fields.payload.configParams.redFlagOffThreshold <= MAX_RED_FLAG_OFF_THRESH &&
            pMsg->fields.payload.configParams.redFlagOffThreshold > pMsg->fields.payload.configParams.redFlagOnThreshold)
    {
        ASP_TransmitAck((uint8_t)ASP_CONFIG_MSG_ID);

        //pass the configs to the APP to handle
        APP_handleConfigs(pMsg->fields.payload.configParams.wakeIntervalInDays, pMsg->fields.payload.configParams.strokeAlgIsOn,
                          pMsg->fields.payload.configParams.redFlagOnThreshold, pMsg->fields.payload.configParams.redFlagOffThreshold);
    }
    else
    {
        //nack + report if the AM sent bad configs
        ASP_HandleErroneousMsg();
    }
}


// A command message was received.  Identify the command and respond with an ACK
void ASP_HandleCommandMsg(asp_msg_t * p_msg)
{
    switch (p_msg->fields.payload.cmd.cmd)
    {
        case CMD_GET_STATUS:
        {
            ASP_TransmitStatus();
            break;
        }
        case CMD_SW_RESET:
        {
            ASP_TransmitAck(CMD_SW_RESET);
            HW_PerformSwReset();
            break;
        }
        case CMD_GET_ENTRIES_IN_LOG:
        {
            ASP_TransmitBytesInSensorDataLog();
            break;
        }
        case CMD_GET_ATTN_SRC:
        {
            APP_handleAttnSourceRequest();
            break;
        }
        case CMD_ACTIVATE:
        {
            ASP_TransmitAck(CMD_ACTIVATE);
            APP_handleActivateCmd();
            break;
        }
        case CMD_DEACTIVATE:
        {
            ASP_TransmitAck(CMD_DEACTIVATE);
            APP_handleDeactivateCmd();
            break;
        }

        case CMD_INCREMENT_SENSOR_DATA_TAIL:
        {
            ASP_TransmitAck(CMD_INCREMENT_SENSOR_DATA_TAIL);
            APP_handleIncrementSensorDataCmd();
            break;
        }

        case CMD_HW_RESET:
        {
            ASP_TransmitAck(CMD_HW_RESET);
            APP_handleHwResetCommand();
            break;
        }
        case CMD_RESET_ALARMS:
        {
            ASP_TransmitAck(CMD_RESET_ALARMS);
            APP_handleResetAlarmsCommand();
            break;
        }
        default:
        {
            break;
        }
    }
}

void ASP_HandleAttnSourceAckMsg(asp_attn_source_payload_t *pMsg)
{
    //pass up to the APP to clear sources/handle
    ASP_TransmitAck(ASP_ATTN_SRC_ACK_MSG_ID);
    APP_handleAttnSourceAck(pMsg);
}

// Transmit the status message.
void ASP_TransmitStatus(void)
{
     asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_STATUS_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_STATUS_MSG_ID;

    p_msg->fields.payload.status.resetState = APP_getResetState();
    p_msg->fields.payload.status.activatedState = APP_getState();
    p_msg->fields.payload.status.ssmFwVersion.fwMaj = VERSION_MAJOR;
    p_msg->fields.payload.status.ssmFwVersion.fwMin = VERSION_MINOR;
    p_msg->fields.payload.status.ssmFwVersion.fwBuild = VERSION_BUILD;
    p_msg->fields.payload.status.errorBits = APP_getErrorBits();
    p_msg->fields.payload.status.timestamp = HW_RTC_GetEpochTime();
    p_msg->fields.payload.status.voltageMv = HW_BAT_GetVoltage();
    p_msg->fields.payload.status.powerRemainingPercent = 50;
    p_msg->fields.payload.status.breakdown = false;
    p_msg->fields.payload.status.magnetDetected = APP_ALGO_isMagnetPresent();
    p_msg->fields.payload.status.activatedDate = APP_NVM_Custom_GetActivatedDate();
    p_msg->fields.payload.status.unexpectedResetCount = APP_NVM_Custom_GetUnexpectedResetCount();
    p_msg->fields.payload.status.timeLastReset = APP_NVM_Custom_GetTimestampLastUnexpectedReset();

    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));
}


void ASP_TransmitAttnSourceList(asp_attn_source_payload_t *pList)
{
    asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_ATTN_SRC_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_ATTN_SRC_MSG_ID;

    memcpy(&(p_msg->fields.payload.attnSource), pList, sizeof(asp_attn_source_payload_t));

    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));
}

void ASP_TransmitAck(uint8_t ackId)
{
    asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_ACK_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_ACK_MSG_ID;

    p_msg->fields.payload.ack.id = ackId;

    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));
}

void ASP_TransmitBytesInSensorDataLog(void)
{
    asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_NUM_DATA_ENTRIES_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_NUM_DATA_ENTRIES_MSG_ID;

    p_msg->fields.payload.entriesInDataLog.numEntries = APP_NVM_Custom_GetSensorDataNumEntries();
    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));
}

void ASP_TransmitSensorDataLog(APP_NVM_SENSOR_DATA_T * p_sensorData)
{
    asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_SENSOR_DATA_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_SENSOR_DATA_MSG_ID;

    memcpy(&(p_msg->fields.payload.sensorData), p_sensorData, sizeof(asp_sensor_data_payload_t));

    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));
}

void ASP_HandleErroneousMsg(void)
{
    //Send a NACK
    asp_msg_t * p_msg = ASP_GetTxBuffer();

    p_msg->fields.startFrame = ASP_START_FRAME_MAGIC;
    p_msg->fields.payloadLen = ASP_NACK_PAYLOAD_BYTES;
    p_msg->fields.messageID = ASP_NACK_MSG_ID;

    p_msg->fields.checksum = ASP_ComputeChecksum(p_msg);
    p_msg->bytes[(p_msg->fields.payloadLen + ASP_HEADER_BYTES)] = p_msg->fields.checksum;

    uC_SPI_Tx(((uint8_t *)(&(p_msg->bytes))), (p_msg->fields.payloadLen + ASP_TOTAL_OVERHEAD_BYTES));

    //now pass up to the APP
    APP_indicateInvalidSpiMsg();
}


#endif // BUILD_SSM
