/*
================================================================================================#=
Module:   Memory Map

Description:
    External FLASH memory layout

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef DEVICE_DRIVERS_MEMORYMAP_H_
#define DEVICE_DRIVERS_MEMORYMAP_H_

//The FLASH memory layout of the NAND flash chip
//Sections must start and end on page boundaries

#define APP_MEM_ADR_CONFIG_START                         0x00042000 //block 2
#define APP_MEM_ADR_CONFIG_END                           0x00044048

#define APP_MEM_ADR_SENSOR_DATA_LOGS_START               0x00080000 //block 3 - 5
#define APP_MEM_ADR_SENSOR_DATA_LOGS_END                 0x000BFFFF
#define APP_MEM_SIZE_OF_SENSOR_DATA_SECTION              (APP_MEM_ADR_SENSOR_DATA_LOGS_END - APP_MEM_ADR_SENSOR_DATA_LOGS_START)


#define APP_MEM_FW_REGISTRY_START                        0x000C0000
#define APP_MEM_FW_REGISTRY_END                          0x000DFFFF

#define APP_MEM_ADR_FW_APPLICATION_AM_A_START            0x00160000 //block 10 - 16
#define APP_MEM_ADR_FW_APPLICATION_AM_A_END              0x0023FFFF


#define APP_MEM_ADR_FW_APPLICATION_SSM_A_START           0x00240000
#define APP_MEM_ADR_FW_APPLICATION_SSM_A_END             0x0027FFFF

#define APP_MEM_ADR_FW_APPLICATION_AM_B_START            0x00280000
#define APP_MEM_ADR_FW_APPLICATION_AM_B_END              0x0034FFFF


#define APP_MEM_ADR_FW_APPLICATION_SSM_B_START           0x00350000
#define APP_MEM_ADR_FW_APPLICATION_SSM_B_END             0x0038FFFF

#define APP_MEM_NUM_SECTIONS                             7

#define APP_MEM_ADR_MAGIC_VALUE                          0x00400000

//default values
//define min, max, and default values for configs:
#define MIN_WAKE_AM_RATE_DAYS                           1
#define MAX_WAKE_AM_RATE_DAYS                           56

#define MIN_MFG_COMPLETE_WAIT_SECS                      30
#define MAX_MFG_COMPLETE_WAIT_SECS                      600
#define DEFAULT_MFG_COMPLETE_WAIT_SECS                  300

#define DEFAULT_WAKE_AM_RATE_DAYS                       28

#define MIN_HIGH_LEVEL_STATE_VAL                        0
#define MAX_HIGH_LEVEL_STATE_VAL                        2
#define DEFAULT_HIGH_LEVEL_STATE_VAL                    2

//allow 30 resets before we decide to reload the SSM (if there have been this many resets then this is likely
//the watchdog resetting us)
//picking 30 to account for resets during CI testing/etc
#define MAX_RESETS_SINCE_LP_MODE                        30

#define MAX_GPS_RETRIES                                 3


#endif /* DEVICE_DRIVERS_MEMORYMAP_H_ */
