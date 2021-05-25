/**************************************************************************************************
* \file     am-ssm-spi-protocol.h
* \brief    Shared message payloads and structures for AM/SSM communication
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
***************************************************************************************************/
#ifndef HANDLERS_AM_SSM_SPI_PROTOCOL_H_
#define HANDLERS_AM_SSM_SPI_PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include "APP_NVM_Cfg_Shared.h"
// AM INCLUDES ---------------------------------------------------------
#ifdef AM_BUILD

// SSM INCLUDES --------------------------------------------------------
#elif defined(SSM_BUILD)
#include "uC_SPI.h"
#include "HW_TERM.h"
#else

#error "Must define AM_BUILD or SSM_BUILD"

#endif

#define BIT_0       0x01
#define BIT_1       0x02
#define BIT_2       0x04
#define BIT_3       0x08
#define BIT_4       0x10
#define BIT_5       0x20
#define BIT_6       0x40
#define BIT_7       0x80
#define BIT_8       0x100
#define BIT_9       0x200
#define BIT_10      0x400
#define BIT_11      0x800
#define BIT_12      0x1000


//return types for protocol related function calls
typedef enum
{
    ERRONEOUS_MSG,
    INVALID_CS,
    INVALID_MSG_ID,
    INVALID_LEN,
    VALID_MSG,
    NACKED_MSG,
    TIMEOUT,
    SUCCESSFUL_REQUEST,
    COM_SUCCESS,
    BAD_REQUEST,
}aspMessageCode_t;

/******************************************************************************
 *  ACK 0x91
 ******************************************************************************/

typedef struct __attribute__ ((packed)) asp_ack_payload
{
    uint8_t id;
}asp_ack_payload_t;


/******************************************************************************
 * SSM Comamand 0x11
 ******************************************************************************/

typedef enum asp_commands
{
    CMD_HW_RESET = (uint8_t) 0,
    CMD_SW_RESET = (uint8_t) 1,
    CMD_GET_STATUS = (uint8_t) 2,
    CMD_GET_ATTN_SRC = (uint8_t) 3,
    CMD_GET_ENTRIES_IN_LOG = (uint8_t) 4,
    CMD_PREP_FW_UPDATE = (uint8_t) 5,
    CMD_DEACTIVATE = (uint8_t) 6,
    CMD_ACTIVATE = (uint8_t) 7,
    CMD_RESET_NVM_SENSOR_DATA = (uint8_t) 8,
    CMD_INCREMENT_SENSOR_DATA_TAIL = (uint8_t) 9,
    CMD_RESET_ALARMS = (uint8_t) 10,
    ASP_NUM_CMDS
}asp_commands_t;


typedef struct __attribute__ ((packed)) asp_command_payload
{
    uint8_t cmd;
}asp_command_payload_t;

/******************************************************************************
 *  Configuration parameters 0x10
 ******************************************************************************/

typedef struct __attribute__ ((packed)) asp_config_param_payload
{
    uint16_t wakeIntervalInDays;
    bool strokeAlgIsOn;
    uint16_t redFlagOnThreshold;
    uint16_t redFlagOffThreshold;
    uint16_t reservedConfig_4;
    uint16_t reservedConfig_5;
}asp_config_param_payload_t;

/******************************************************************************
 *  Set RTC Time 0x12
 ******************************************************************************/

typedef struct __attribute__ ((packed)) asp_set_rtc
{
    uint32_t RTC_time __attribute__ ((packed, aligned(1)));
}asp_set_rtc_payload_t;

/******************************************************************************
 * SSM error bits
 ******************************************************************************/


//Active error bit flags - application level. Algorithm bits are defined in APP_ALGO.h and
//offset by 13
#define MAG_ERROR               BIT_0
#define TEMP_HUMID_ERROR        BIT_1
#define CAPSENSE_ERROR          BIT_2
#define EEPROM_READ_ERROR       BIT_3
#define EEPROM_WRITE_ERROR      BIT_4
#define RTC_COMM_ERROR          BIT_5
#define SPI_ERROR               BIT_6
#define I2C_ERROR               BIT_7
#define CAP_SENSE_NO_DATA       BIT_8
#define CAP_SENSE_SAMPLE_ERROR  BIT_9
#define NO_RTC_TIME             BIT_10
#define AM_NOT_RESPONSIVE       BIT_11
#define CURRENT_DRAW_HIGH       BIT_12


/******************************************************************************
 * SSM Status 0x20
 ******************************************************************************/

typedef enum asp_state
{
    STATE_POR = (uint8_t) 0,
    STATE_SWR = (uint8_t) 1,
    STATE_OK =  (uint8_t) 2,
    STATE_ERR = (uint8_t) 99
}reset_state_t;

typedef enum
{
    ACTIVATED = (uint8_t) 0,
    DEACTIVATED = (uint8_t) 1,
    FAULT =  (uint8_t) 2,
}app_state_t;

typedef struct __attribute__ ((packed)) asp_fw_ver
{
    uint8_t fwMaj;
    uint8_t fwMin;
    uint8_t fwBuild;
}asp_fw_ver_t;

typedef struct __attribute__ ((packed)) asp_status_payload
{
    uint8_t resetState;
    uint8_t activatedState;
    asp_fw_ver_t ssmFwVersion;
    uint32_t errorBits;
    uint32_t timestamp __attribute__ ((packed, aligned(1)));
    uint32_t voltageMv;
    uint32_t powerRemainingPercent;
    bool magnetDetected;
    bool breakdown;
    uint32_t activatedDate;
    uint32_t unexpectedResetCount;
    uint32_t timeLastReset;
}asp_status_payload_t;


/******************************************************************************
 * 0x 21 Sensor Data log Entry
 ******************************************************************************/

typedef APP_NVM_SENSOR_DATA_T asp_sensor_data_entry_t;

typedef struct __attribute__ ((packed)) asp_data_log_payload
{
    asp_sensor_data_entry_t entry;
}asp_sensor_data_payload_t;


/******************************************************************************
 *  0x 13 - Get Next data entry. If we want to expand this to multiple entries,
 *  then we will need to define buffer space large enough to hold more entries
 *  (they are about 175 bytes/entry as of now)
 ******************************************************************************/

typedef struct __attribute__ ((packed)) asp_get_data_entries_payload
{
    uint16_t getNextEntry;
}asp_get_data_entries_payload_t;


/******************************************************************************
 * 0x24 - Number of sensor data log entries currently stored
 ******************************************************************************/

typedef struct __attribute__ ((packed)) asp_number_data_entries_payload
{
    uint16_t numEntries;
}asp_number_data_entries_payload_t;

/******************************************************************************
 * Attention source list 0x23, Also used for the 0x25 Attn source ACK
 ******************************************************************************/

#define ASP_ACTIVATE                    BIT_0
#define ASP_REQUEST_TIME                BIT_1
#define ASP_CHECK_IN_ACTIVATED          BIT_2
#define ASP_CHECK_IN_DEACTIVATED        BIT_3
#define ASP_SSM_ERROR                   BIT_4

typedef struct __attribute__ ((packed)) asp_attn_source_payload
{
    uint8_t attnSourceList;
}asp_attn_source_payload_t;

/***********************************************************************************
 *          Payload sizes/command IDs
 ***********************************************************************************/

#define ASP_START_FRAME_MAGIC                     ((uint8_t) 0xA5)
#define ASP_FREE_BUFFER_MARKER                    (0x5A)
#define ASP_MAX_MSGS                              (3u)
#define ASP_MAX_PAYLOAD                           (210u)
#define ASP_HEADER_BYTES                          ((uint8_t)3u)
#define ASP_CHKSUM_BYTES                          ((uint8_t)1u)
#define ASP_TOTAL_OVERHEAD_BYTES                  ((uint8_t)ASP_HEADER_BYTES + ASP_CHKSUM_BYTES)

//AM -> SSM
#define ASP_CONFIG_MSG_ID                         (0x10)
#define ASP_CONFIG_PAYLOAD_BYTES                  (sizeof(asp_config_param_payload_t))
#define ASP_COMMAND_MSG_ID                        (0x11)
#define ASP_COMMAND_PAYLOAD_BYTES                 (sizeof(asp_command_payload_t))
#define ASP_SET_RTC_MSG_ID                        (0x12)
#define ASP_SET_RTC_PAYLOAD_BYTES                 (sizeof(asp_set_rtc_payload_t))
#define ASP_GET_SENSOR_DATA_ENTRIES_MSG_ID        (0x13)
#define ASP_GET_SENSOR_DATA_ENTRIES_PAYLOAD_BYTES (sizeof(asp_get_data_entries_payload_t))
#define ASP_ATTN_SRC_ACK_MSG_ID                   (0x25)
#define ASP_ATTN_SRC_ACK_PAYLOAD_BYTES            (sizeof(asp_attn_source_payload_t))

//SSM -> AM
#define ASP_STATUS_MSG_ID                         (0x20)
#define ASP_STATUS_PAYLOAD_BYTES                  (sizeof(asp_status_payload_t))
#define ASP_SENSOR_DATA_MSG_ID                    (0x21)
#define ASP_SENSOR_DATA_PAYLOAD_BYTES             (sizeof(asp_sensor_data_payload_t))
#define ASP_NUM_DATA_ENTRIES_MSG_ID               (0x24)
#define ASP_NUM_DATA_ENTRIES_PAYLOAD_BYTES        (sizeof(asp_number_data_entries_payload_t))
#define ASP_ATTN_SRC_MSG_ID                       (0x23)
#define ASP_ATTN_SRC_PAYLOAD_BYTES                (sizeof(asp_attn_source_payload_t))
#define ASP_ACK_MSG_ID                            (0x91)
#define ASP_ACK_PAYLOAD_BYTES                     (sizeof(asp_ack_payload_t))
#define ASP_NACK_MSG_ID                           (0x92)
#define ASP_NACK_PAYLOAD_BYTES                    (0u)


#define ASP_NORESPONSE_ID                         (0x00)


/******************************************************************************
 * SSM/AP SPI Protocol Message
 ******************************************************************************/
typedef union __attribute__ ((packed)) {
    asp_ack_payload_t          ack;
    asp_command_payload_t      cmd;
    asp_status_payload_t       status;
    asp_number_data_entries_payload_t    entriesInDataLog;
    asp_sensor_data_payload_t     sensorData;
    asp_get_data_entries_payload_t      getLog;
    asp_set_rtc_payload_t      setRTC;
    asp_attn_source_payload_t  attnSource;
    asp_config_param_payload_t configParams;
    uint8_t                    bytes[ASP_MAX_PAYLOAD];
}asp_payload_t;

typedef union __attribute__ ((packed)) {
    struct   {
        uint8_t startFrame;                                      /**< Start frame byte */
        uint8_t payloadLen;                                      /**< payload length */
        uint8_t messageID;                                       /**< message ID */
        asp_payload_t payload;       /**< message payload */
        uint8_t checksum;           /**< checksum */
        uint8_t responseID;         /**< response ID */
    }fields;
    uint8_t bytes[sizeof(asp_payload_t) + 4];   /**< message bytes */
}asp_msg_t;


// AM FUNCTIONS ---------------------------------------------------------
#ifdef AM_BUILD
extern aspMessageCode_t ASP_SendCommandMsg(asp_commands_t cmd,  uint8_t response_id, uint8_t expected_bytes, uint8_t *rspBuffer, uint8_t *rspLen);
extern aspMessageCode_t ASP_GetSSMStatus(asp_status_payload_t * ssmStat);
extern aspMessageCode_t ASP_GetAttnSrcList(asp_attn_source_payload_t * attnSources);
extern aspMessageCode_t ASP_CommandReset(void);
extern aspMessageCode_t ASP_GetSensorDataNumEntries(asp_number_data_entries_payload_t * entriesInLog);
extern aspMessageCode_t ASP_GetSensorData(uint16_t entriesToGet, asp_sensor_data_entry_t *entry);
extern aspMessageCode_t ASP_SensorDataStoredToFlash(void);
extern aspMessageCode_t ASP_SetTime(uint32_t time);
extern aspMessageCode_t ASP_SendAttnSrcAckMsg(asp_attn_source_payload_t *pMsg);
extern aspMessageCode_t ASP_SendActivate(void);
extern aspMessageCode_t ASP_SendDeActivate(void);
extern aspMessageCode_t ASP_ProcessIncomingBuffer(uint8_t *bytes, uint16_t bufferLen, asp_msg_t *formattedMsg);
extern aspMessageCode_t ASP_SendHwReset(void);
extern aspMessageCode_t ASP_SendResetAlarmsCmd(void);
extern aspMessageCode_t ASP_SendConfigs(uint16_t transmissionRateDays, bool  strokeAlgIsOn,  uint16_t redFlagOnThresh, uint16_t redFlagOffThresh);

// SSM FUNCTIONS --------------------------------------------------------
#elif defined(SSM_BUILD)

extern void ASP_SSM_Periodic(void);
extern void ASP_HandleCommandMsg(asp_msg_t * p_msg);
extern void ASP_TransmitBytesInSensorDataLog(void);
extern void ASP_HandleGetSensorDataMsg(void);
extern void ASP_TransmitSensorDataLog(APP_NVM_SENSOR_DATA_T * p_sensorData);
extern void ASP_HandleSetRTCMsg(asp_msg_t * p_msg);
extern void ASP_TransmitAttnSourceList(asp_attn_source_payload_t *pList);
extern void ASP_HandleAttnSourceAckMsg(asp_attn_source_payload_t *pMsg);
extern void ASP_TransmitAck(uint8_t ackId);
extern void ASP_HandleErroneousMsg(void);
extern void ASP_HandleConfigMsg(asp_msg_t *pMsg);
#else

#error "Must define AM_BUILD or SSM_BUILD"

#endif

// SHARED FUNCTIONS -----------------------------------------------------
extern uint8_t ASP_ComputeChecksum(asp_msg_t * p_msg);
extern void ASP_ProcessIncomingByte(uint8_t ubByte);
extern asp_msg_t * ASP_GetTxBuffer(void);

#endif /* HANDLERS_AM_SSM_SPI_PROTOCOL_H_ */
