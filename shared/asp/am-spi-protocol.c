/**************************************************************************************************
* \file     am-ssm-spi-protocol.c
* \brief    Protocol functionality specific to the AM.
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

#ifdef AM_BUILD

#include "am-ssm-spi-protocol.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "logTypes.h"
#include "spi.h"
#include <string.h>
#include "CLI.h"

static asp_msg_t Tx_Msg = {};
static asp_msg_t Rx_Msg = {};
static uint8_t rsp[100];
static uint8_t rspLen = 0;

aspMessageCode_t ASP_SendCommandMsg(asp_commands_t cmd,  uint8_t response_id, uint8_t expected_bytes, uint8_t *rspBuffer, uint8_t *rspLen);
aspMessageCode_t ASP_GetSSMStatus(asp_status_payload_t * ssmStat);
aspMessageCode_t ASP_GetAttnSrcList(asp_attn_source_payload_t * attnSources);
aspMessageCode_t ASP_CommandReset(void);
aspMessageCode_t ASP_SendActivate(void);
aspMessageCode_t ASP_SendDeActivate(void);
aspMessageCode_t ASP_GetSensorDataNumEntries(asp_number_data_entries_payload_t * entriesInLog);
aspMessageCode_t ASP_GetSensorData(uint16_t entriesToGet, asp_sensor_data_entry_t *entry);
aspMessageCode_t ASP_SensorDataStoredToFlash(void);
aspMessageCode_t ASP_SetTime(uint32_t time);
aspMessageCode_t ASP_SendConfigs(uint16_t transmissionRateDays, bool strokeAlgIsOn, uint16_t redFlagOnThresh, uint16_t redFlagOffThresh);
aspMessageCode_t ASP_SendAttnSrcAckMsg(asp_attn_source_payload_t *pMsg);

// Transmit the provided command message with the provided to command.
aspMessageCode_t ASP_SendCommandMsg(asp_commands_t cmd,  uint8_t response_id, uint8_t expected_bytes, uint8_t *rspBuffer, uint8_t *rspLen)
{
    spiData_t tx_data;
    spiData_t rx_data;
    aspMessageCode_t result = BAD_REQUEST;

    tx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_COMMAND_PAYLOAD_BYTES);
    tx_data.pChar = (uint8_t*)&Tx_Msg;

    rx_data.length = expected_bytes;
    rx_data.pChar = (uint8_t*)&Rx_Msg;

    // Build a command message and transmit it.
    if ( cmd < ASP_NUM_CMDS )
    {
        Tx_Msg.fields.startFrame = ASP_START_FRAME_MAGIC;
        Tx_Msg.fields.payloadLen = ASP_COMMAND_PAYLOAD_BYTES;
        Tx_Msg.fields.messageID = ASP_COMMAND_MSG_ID;
        Tx_Msg.fields.payload.cmd.cmd = cmd;
        Tx_Msg.fields.checksum = (uint8_t) ASP_ComputeChecksum(&Tx_Msg);
        Tx_Msg.fields.responseID = response_id;
        Tx_Msg.fields.payload.bytes[Tx_Msg.fields.payloadLen] = Tx_Msg.fields.checksum;  // Move checksum to end of payload.

        if ( SPI_ssmTransfer(&tx_data, &rx_data, OpsInitTransfer) != spiSuccess )
        {
            elogError("COMMAND MESSAGE FAILED");
            result = TIMEOUT;
        }
        else
        {
            //put the response into the provided buffer
            memcpy(rspBuffer, rx_data.pChar, rx_data.length);

            *rspLen = rx_data.length;
            result = COM_SUCCESS;
        }
    }

    return result;
}

aspMessageCode_t ASP_GetSensorData(uint16_t entriesToGet, asp_sensor_data_entry_t *entry)
{
    spiData_t tx_data;
    spiData_t rx_data;
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    tx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_GET_SENSOR_DATA_ENTRIES_PAYLOAD_BYTES);
    tx_data.pChar = (uint8_t*)&Tx_Msg;

    rx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_SENSOR_DATA_PAYLOAD_BYTES);
    rx_data.pChar = (uint8_t*)&Rx_Msg;

    Tx_Msg.fields.responseID = ASP_SENSOR_DATA_MSG_ID;

    // Build message and transmit it.
    Tx_Msg.fields.startFrame = ASP_START_FRAME_MAGIC;
    Tx_Msg.fields.payloadLen = ASP_GET_SENSOR_DATA_ENTRIES_PAYLOAD_BYTES;
    Tx_Msg.fields.messageID = ASP_GET_SENSOR_DATA_ENTRIES_MSG_ID;
    Tx_Msg.fields.payload.getLog.getNextEntry = entriesToGet;
    Tx_Msg.fields.checksum = (uint8_t) ASP_ComputeChecksum(&Tx_Msg);
    Tx_Msg.fields.payload.bytes[Tx_Msg.fields.payloadLen] = Tx_Msg.fields.checksum;  // Move checksum to end of payload.

    if ( SPI_ssmTransfer(&tx_data, &rx_data, OpsInitTransfer) != spiSuccess )
    {
        elogError("Get Data message failed");
        result = TIMEOUT;
    }
    else
    {
        result = ASP_ProcessIncomingBuffer(rx_data.pChar, rx_data.length, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.messageID == ASP_SENSOR_DATA_MSG_ID )
            {
                *entry = formatted.fields.payload.sensorData.entry;
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

aspMessageCode_t ASP_SensorDataStoredToFlash(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_INCREMENT_SENSOR_DATA_TAIL, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES) , (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
       //unpack the rsp
       result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

       if (result == VALID_MSG)
       {
          if( formatted.fields.payload.ack.id == CMD_INCREMENT_SENSOR_DATA_TAIL )
          {
              result = SUCCESSFUL_REQUEST;
          }
          else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
          {
              result = NACKED_MSG;
          }
          else
          {
              result = INVALID_MSG_ID;
          }
       }
    }

    return result;

}

aspMessageCode_t ASP_SetTime(uint32_t time)
{
    spiData_t tx_data;
    spiData_t rx_data;
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    tx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_SET_RTC_PAYLOAD_BYTES);
    tx_data.pChar = (uint8_t*)&Tx_Msg;

    rx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES);
    rx_data.pChar = (uint8_t*)&Rx_Msg;

    Tx_Msg.fields.responseID = ASP_ACK_MSG_ID;

    // Build message and transmit it.
    Tx_Msg.fields.startFrame = ASP_START_FRAME_MAGIC;
    Tx_Msg.fields.payloadLen = ASP_SET_RTC_PAYLOAD_BYTES;
    Tx_Msg.fields.messageID = ASP_SET_RTC_MSG_ID;
    Tx_Msg.fields.payload.setRTC.RTC_time = time;

    Tx_Msg.fields.checksum = (uint8_t) ASP_ComputeChecksum(&Tx_Msg);
    Tx_Msg.fields.payload.bytes[Tx_Msg.fields.payloadLen] = Tx_Msg.fields.checksum;  // Move checksum to end of payload.

    if ( SPI_ssmTransfer(&tx_data, &rx_data, OpsInitTransfer) != spiSuccess )
    {
        elogError("SET TIME FAILED TO SEND/RX");
        result = TIMEOUT;
    }
    else
    {
        result = ASP_ProcessIncomingBuffer(rx_data.pChar, rx_data.length, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.payload.ack.id == ASP_SET_RTC_MSG_ID )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

//Send new config message. There is space in the msg if we want to add more configs down the road:
aspMessageCode_t ASP_SendConfigs(uint16_t transmissionRateDays, bool strokeAlgIsOn, uint16_t redFlagOnThresh, uint16_t redFlagOffThresh)
{
    spiData_t tx_data;
    spiData_t rx_data;
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    tx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_CONFIG_PAYLOAD_BYTES);
    tx_data.pChar = (uint8_t*)&Tx_Msg;

    rx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES);
    rx_data.pChar = (uint8_t*)&Rx_Msg;

    Tx_Msg.fields.responseID = ASP_ACK_MSG_ID;

    // Build message and transmit it.
    Tx_Msg.fields.startFrame = ASP_START_FRAME_MAGIC;
    Tx_Msg.fields.payloadLen = ASP_CONFIG_PAYLOAD_BYTES;
    Tx_Msg.fields.messageID = ASP_CONFIG_MSG_ID;
    Tx_Msg.fields.payload.configParams.wakeIntervalInDays = transmissionRateDays;
    Tx_Msg.fields.payload.configParams.strokeAlgIsOn = strokeAlgIsOn;
    Tx_Msg.fields.payload.configParams.redFlagOnThreshold = redFlagOnThresh;
    Tx_Msg.fields.payload.configParams.redFlagOffThreshold = redFlagOffThresh;
    Tx_Msg.fields.payload.configParams.reservedConfig_4 = 0;
    Tx_Msg.fields.payload.configParams.reservedConfig_5 = 0;

    Tx_Msg.fields.checksum = (uint8_t) ASP_ComputeChecksum(&Tx_Msg);
    Tx_Msg.fields.payload.bytes[Tx_Msg.fields.payloadLen] = Tx_Msg.fields.checksum;  // Move checksum to end of payload.

    if ( SPI_ssmTransfer(&tx_data, &rx_data, OpsInitTransfer) != spiSuccess )
    {
        elogError("CONFIG FAILED TO SEND/RX");
        result = TIMEOUT;
    }
    else
    {
        result = ASP_ProcessIncomingBuffer(rx_data.pChar, rx_data.length, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.payload.ack.id == ASP_CONFIG_MSG_ID )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}


aspMessageCode_t ASP_SendAttnSrcAckMsg(asp_attn_source_payload_t *pMsg)
{
    spiData_t tx_data;
    spiData_t rx_data;
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;


    tx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_ATTN_SRC_ACK_PAYLOAD_BYTES);
    tx_data.pChar = (uint8_t*)&Tx_Msg;

    rx_data.length = (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES);
    rx_data.pChar = (uint8_t*)&Rx_Msg;

    Tx_Msg.fields.responseID = ASP_ACK_MSG_ID;

    // Build message and transmit it.
    Tx_Msg.fields.startFrame = ASP_START_FRAME_MAGIC;
    Tx_Msg.fields.payloadLen = ASP_ATTN_SRC_ACK_PAYLOAD_BYTES;
    Tx_Msg.fields.messageID = ASP_ATTN_SRC_ACK_MSG_ID;
    Tx_Msg.fields.payload.attnSource.attnSourceList = pMsg->attnSourceList;

    Tx_Msg.fields.checksum = (uint8_t) ASP_ComputeChecksum(&Tx_Msg);
    Tx_Msg.fields.payload.bytes[Tx_Msg.fields.payloadLen] = Tx_Msg.fields.checksum;  // Move checksum to end of payload.

    if ( SPI_ssmTransfer(&tx_data, &rx_data, OpsInitTransfer) != spiSuccess )
    {
        elogError("SEND ATTN SRC ACK MSG FAILED");
        result = TIMEOUT;
    }
    else
    {
        result = ASP_ProcessIncomingBuffer(rx_data.pChar, rx_data.length, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.payload.ack.id == ASP_ATTN_SRC_ACK_MSG_ID )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

aspMessageCode_t ASP_CommandReset(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_SW_RESET, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
       //unpack the rsp
       result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

       if (result == VALID_MSG)
       {
          if( formatted.fields.payload.ack.id == CMD_SW_RESET )
          {
              result = SUCCESSFUL_REQUEST;
          }
          else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
          {
              result = NACKED_MSG;
          }
          else
          {
              result = INVALID_MSG_ID;
          }
       }
    }

    return result;
}

aspMessageCode_t ASP_SendActivate(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_ACTIVATE, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
        //unpack the rsp
        result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.payload.ack.id == CMD_ACTIVATE )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

aspMessageCode_t ASP_SendDeActivate(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_DEACTIVATE, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES) , (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
       //unpack the rsp
       result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

       if (result == VALID_MSG)
       {
          if( formatted.fields.payload.ack.id == CMD_DEACTIVATE )
          {
              result = SUCCESSFUL_REQUEST;
          }
          else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
          {
              result = NACKED_MSG;
          }
          else
          {
              result = INVALID_MSG_ID;
          }
       }
    }

    return result;
}

aspMessageCode_t ASP_GetSensorDataNumEntries(asp_number_data_entries_payload_t * entriesInLog)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

   result = ASP_SendCommandMsg(CMD_GET_ENTRIES_IN_LOG, ASP_NUM_DATA_ENTRIES_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_NUM_DATA_ENTRIES_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

   if (result == COM_SUCCESS )
   {
       //unpack the rsp
       result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

       if (result == VALID_MSG)
       {
           if ( formatted.fields.messageID == ASP_NUM_DATA_ENTRIES_MSG_ID )
           {
               *entriesInLog = formatted.fields.payload.entriesInDataLog;
               result = SUCCESSFUL_REQUEST;
           }
           else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
           {
               result = NACKED_MSG;
           }
           else
           {
               result = INVALID_MSG_ID;
           }
       }
   }

   return result;
}

aspMessageCode_t ASP_GetSSMStatus(asp_status_payload_t * ssmStat)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_GET_STATUS, ASP_STATUS_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_STATUS_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
        //unpack the rsp
        result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

        if (result == VALID_MSG)
        {
            if ( formatted.fields.messageID == ASP_STATUS_MSG_ID )
            {
                *ssmStat = formatted.fields.payload.status;
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}


aspMessageCode_t ASP_SendHwReset(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_HW_RESET, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
        //unpack the rsp
        result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

        if (result == VALID_MSG)
        {
            if ( formatted.fields.payload.ack.id == CMD_HW_RESET )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

aspMessageCode_t ASP_SendResetAlarmsCmd(void)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_RESET_ALARMS, ASP_ACK_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ACK_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
        //unpack the rsp
        result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

        if (result == VALID_MSG)
        {
            if ( formatted.fields.payload.ack.id == CMD_RESET_ALARMS )
            {
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

aspMessageCode_t ASP_GetAttnSrcList(asp_attn_source_payload_t * attnSources)
{
    aspMessageCode_t result = BAD_REQUEST;
    asp_msg_t formatted;

    result = ASP_SendCommandMsg(CMD_GET_ATTN_SRC, ASP_ATTN_SRC_MSG_ID, (ASP_TOTAL_OVERHEAD_BYTES + ASP_ATTN_SRC_PAYLOAD_BYTES), (uint8_t*)&rsp, &rspLen);

    if (result == COM_SUCCESS )
    {
        //unpack the rsp
        result = ASP_ProcessIncomingBuffer((uint8_t*)&rsp, rspLen, &formatted);

        if (result == VALID_MSG)
        {
            if( formatted.fields.messageID == ASP_ATTN_SRC_MSG_ID )
            {
                *attnSources = formatted.fields.payload.attnSource;
                result = SUCCESSFUL_REQUEST;
            }
            else if ( formatted.fields.messageID == ASP_NACK_MSG_ID )
            {
                result = NACKED_MSG;
            }
            else
            {
                result = INVALID_MSG_ID;
            }
        }
    }

    return result;
}

#endif //AM_BUILD
