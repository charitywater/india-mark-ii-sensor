
/*************************************************************************************************
* \file     am-ssm-spi-protocol.c
* \brief    Protocol functionality ahared between the AM and the SSM.
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

#include "am-ssm-spi-protocol.h"
#include <stdint.h>
#include <stdio.h>

typedef enum asp_message_state
{
    ASP_LOOK_FOR_START = 0,   /*< Looking for start. */
    ASP_RECEIVE_LEN,          /*< Receving length */
    ASP_RECEIVE_ID,           /*< Receiving identyifier */
    ASP_RECEIVE_PAYLOAD,      /*< Receiving payloaf */
    ASP_RECEIVE_CHECKSUM      /*< Receiving checksum */
}asp_message_state_t;

static asp_msg_t Tx_Msg = {};

// SHARED FUNCTIONS ---------------------------------------------------------
uint8_t ASP_ComputeChecksum(asp_msg_t * p_msg);
asp_msg_t * ASP_GetTxBuffer(void);
aspMessageCode_t ASP_ProcessIncomingBuffer(uint8_t *bytes, uint16_t bufferLen, asp_msg_t *formattedMsg);


#if defined(SSM_BUILD)

void ASP_ProcessIncomingByte(uint8_t byte);
static void HandleValidMsg(asp_msg_t * p_msg);

static asp_msg_t Rx_Msg = {};

// A message is being or has been received.  Parse the header info and
// checksum and make sure it is good before calling the handler.
void ASP_ProcessIncomingByte(uint8_t byte)
{
	static uint8_t payload_bytes_expected = 0;
	static uint8_t payload_bytes_received = 0;
    static uint8_t receive_state = ASP_LOOK_FOR_START;

    /* - A byte has been received.  State machine to process it. */
	switch ( receive_state )
	{
		case ASP_LOOK_FOR_START:
		{
            /* If we found the magic value, prepare to start saving the message. */
			if ( byte == ASP_START_FRAME_MAGIC )
			{
			    payload_bytes_expected = 0;
				receive_state = ASP_RECEIVE_LEN;
			}
			break;
		}
		case ASP_RECEIVE_LEN:
		{
            /* - if length is valid, look for ID */
			if ( byte <= sizeof(asp_payload_t) )
			{
				Rx_Msg.fields.payloadLen = byte;
				receive_state = ASP_RECEIVE_ID;
			}
			else
			{
                /* - Length is invalid.  Start over. */
			    ASP_HandleErroneousMsg();
				receive_state = ASP_LOOK_FOR_START;
			}
			break;
		}
		case ASP_RECEIVE_ID:
		{
			switch ( byte )
			{
                case ASP_CONFIG_MSG_ID                       :
                case ASP_COMMAND_MSG_ID                      :
                case ASP_SET_RTC_MSG_ID                      :
                case ASP_STATUS_MSG_ID                       :
                case ASP_ATTN_SRC_MSG_ID                     :
                case ASP_ATTN_SRC_ACK_MSG_ID                 :
                case ASP_GET_SENSOR_DATA_ENTRIES_MSG_ID      :
				{
                    /* - Valid ID received */
					Rx_Msg.fields.messageID = byte;

					if ( Rx_Msg.fields.payloadLen > 0 )
					{
                        /* - There is a payload so prepare to receive it.  (NACKs can have zero payload) */
						payload_bytes_expected = Rx_Msg.fields.payloadLen;
						payload_bytes_received = 0;
						receive_state = ASP_RECEIVE_PAYLOAD;
					}
					else
					{
                        /* - No payload, receive checksum */
						receive_state = ASP_RECEIVE_CHECKSUM;
					}
					break;
				}
				default:
				{
                    /* - Invalid ID.  Restart */
				    ASP_HandleErroneousMsg();
					receive_state = ASP_LOOK_FOR_START;
					break;
				}
			}
			break;
		}
		case ASP_RECEIVE_PAYLOAD:
		{
            /* - Receive the expected number of bytes */
			Rx_Msg.fields.payload.bytes[payload_bytes_received] = byte;
			payload_bytes_received++;
			if ( payload_bytes_received >= payload_bytes_expected )
            {
				receive_state = ASP_RECEIVE_CHECKSUM;
            }
			break;
		}
	 	case ASP_RECEIVE_CHECKSUM:
	 	{
             /* Validate the checksum */
	 		if ( ASP_ComputeChecksum(&Rx_Msg) == byte )
	 		{
                 /* - Checksum is good.  Call the handler. */
	 		    HandleValidMsg(&Rx_Msg);
	 		}
	 		else
	 		{
                /* Checksum is incorrect. */
                ASP_HandleErroneousMsg();
	 		}
             /* - Reset state machine.  */
             receive_state = ASP_LOOK_FOR_START;
	 		break;
		}
	}
}


// A validly constructed message has been received.  Now send it to the handler.
static void HandleValidMsg(asp_msg_t * p_msg)
{
    /* Call handler for given message ID */
    if ( p_msg != NULL)
    {
        switch ( p_msg->fields.messageID )
        {
            case ASP_COMMAND_MSG_ID:
            {
                ASP_HandleCommandMsg(p_msg);
                break;
            }
            case ASP_GET_SENSOR_DATA_ENTRIES_MSG_ID:
            {
                ASP_HandleGetSensorDataMsg();
                break;
            }
            case ASP_SET_RTC_MSG_ID:
            {
                ASP_HandleSetRTCMsg(p_msg);
                break;
            }
            case ASP_ATTN_SRC_ACK_MSG_ID:
            {
                ASP_HandleAttnSourceAckMsg((void*)p_msg);
                break;
            }
            case ASP_CONFIG_MSG_ID:
            {
                ASP_HandleConfigMsg(p_msg);
                break;
            }
            default :
            {
                ASP_HandleErroneousMsg();
                break;
            }
        }
    }
}

#endif

// Get a pointer to the transmit buffer.
asp_msg_t * ASP_GetTxBuffer(void)
{
    return &Tx_Msg;
}

#ifdef AM_BUILD
//Pass in a full received message (on the AM) and this function formats the msg and determines
//if it is valid OR the reason that it is an invalid msg
aspMessageCode_t ASP_ProcessIncomingBuffer(uint8_t *bytes, uint16_t bufferLen, asp_msg_t *formattedMsg)
{
    uint8_t payload_bytes_expected = 0;
    uint8_t payload_bytes_received = 0;
    uint8_t receive_state = ASP_LOOK_FOR_START;
    aspMessageCode_t validMsg = VALID_MSG;
    asp_msg_t rxMsg = {};
    uint16_t i = 0;

    for ( i=0; i< bufferLen; i++)
    {
        switch ( receive_state )
        {
            case ASP_LOOK_FOR_START:
            {
                /* If we found the magic value, prepare to start saving the message. */
                if ( bytes[i] == ASP_START_FRAME_MAGIC )
                {
                    payload_bytes_expected = 0;
                    receive_state = ASP_RECEIVE_LEN;
                }
                else
                {
                    //Invalid first byte
                    validMsg = ERRONEOUS_MSG;
                }
                break;
            }
            case ASP_RECEIVE_LEN:
            {
                /* - if length is valid, look for ID */
                if ( bytes[i] <= sizeof(asp_payload_t) )
                {
                    rxMsg.fields.payloadLen = bytes[i];
                    receive_state = ASP_RECEIVE_ID;
                }
                else
                {
                    validMsg = INVALID_LEN;
                }
                break;
            }
            case ASP_RECEIVE_ID:
            {
                switch ( bytes[i] )
                {
                    case ASP_CONFIG_MSG_ID              :
                    case ASP_COMMAND_MSG_ID             :
                    case ASP_SET_RTC_MSG_ID             :
                    case ASP_NUM_DATA_ENTRIES_MSG_ID    :
                    case ASP_STATUS_MSG_ID              :
                    case ASP_ATTN_SRC_MSG_ID            :
                    case ASP_ATTN_SRC_ACK_MSG_ID        :
                    case ASP_SENSOR_DATA_MSG_ID         :
                    case ASP_ACK_MSG_ID                 :
                    case ASP_NACK_MSG_ID                :
                    {
                        /* - Valid ID received */
                        rxMsg.fields.messageID = bytes[i];

                        if ( rxMsg.fields.payloadLen > 0 )
                        {
                            /* - There is a payload so prepare to receive it.  (NACKs can have zero payload) */
                            payload_bytes_expected = rxMsg.fields.payloadLen;
                            payload_bytes_received = 0;
                            receive_state = ASP_RECEIVE_PAYLOAD;
                        }
                        else
                        {
                            /* - No payload, receive checksum */
                            receive_state = ASP_RECEIVE_CHECKSUM;
                        }
                        break;
                    }
                    default:
                    {
                        /* - Invalid ID.  Restart */
                        validMsg = INVALID_MSG_ID;
                        break;
                    }
                }
                break;
            }
            case ASP_RECEIVE_PAYLOAD:
            {
                /* - Receive the expected number of bytes */
                rxMsg.fields.payload.bytes[payload_bytes_received] = bytes[i];
                payload_bytes_received++;
                if ( payload_bytes_received >= payload_bytes_expected )
                {
                    receive_state = ASP_RECEIVE_CHECKSUM;
                }
                break;
            }
            case ASP_RECEIVE_CHECKSUM:
            {
                 /* Validate the checksum */
                if ( ASP_ComputeChecksum(&rxMsg) == bytes[i] )
                {
                     /* - Checksum is good.  */
                    validMsg = VALID_MSG;
                }
                else
                {
                     /* Checksum is incorrect. */
                    validMsg = INVALID_CS;
                }
                break;
            }
        }

        //check msg code before continuing the loop
        if (validMsg != VALID_MSG)
        {
            break;
        }
    }

    *formattedMsg = rxMsg;

    return validMsg;
}

#endif //AM_BUILD

// Compute the 2's compliment checksum of the message.
uint8_t ASP_ComputeChecksum(asp_msg_t * p_msg)
{
    uint8_t checksum = 0;
    uint8_t bytes = 0;

    /* Compute the checksum of the message */
    checksum += p_msg->fields.payloadLen;
    checksum += p_msg->fields.messageID;

    for ( bytes = 0; bytes < p_msg->fields.payloadLen; bytes++ )
    {
        checksum += p_msg->fields.payload.bytes[bytes];
    }
    checksum = (~checksum);
    checksum++;

    return checksum;
}
