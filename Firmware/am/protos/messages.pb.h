/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.0 */

#ifndef PB_MESSAGES_PB_H_INCLUDED
#define PB_MESSAGES_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _eState {
    eState_ACTIVATED = 0,
    eState_DEACTIVATED = 1,
    eState_FAULT = 2
} eState;

/* Struct definitions */
typedef struct _CommonHeader {
    uint32_t productId;
    uint32_t timestamp;
    uint32_t msgNumber;
    uint32_t fwMajor;
    uint32_t fwMinor;
    uint32_t fwBuild;
    bool has_voltage;
    uint32_t voltage;
    bool has_powerRemaining;
    uint32_t powerRemaining;
    bool has_state;
    eState state;
    bool has_activatedDate;
    uint32_t activatedDate;
    bool has_magnetDetected;
    bool magnetDetected;
    bool has_errorBits;
    uint32_t errorBits;
    bool has_numSSMResets;
    uint32_t numSSMResets;
    bool has_lastSSMResetDate;
    uint32_t lastSSMResetDate;
    bool has_numAMResets;
    uint32_t numAMResets;
    bool has_lastAMResetDate;
    uint32_t lastAMResetDate;
    bool has_logs;
    char logs[100];
    bool has_rssi;
    uint32_t rssi;
    bool has_connectTime;
    uint32_t connectTime;
    bool has_mfgComplete;
    bool mfgComplete;
    uint64_t imei;
} CommonHeader;

typedef struct _GpsMessage {
    CommonHeader header;
    bool has_hours;
    uint32_t hours;
    bool has_minutes;
    uint32_t minutes;
    bool has_latitude;
    float latitude;
    bool has_longitude;
    float longitude;
    bool has_altitude;
    float altitude;
    bool has_fixQuality;
    uint32_t fixQuality;
    bool has_satellitesTracked;
    uint32_t satellitesTracked;
    bool has_hdopValue;
    float hdopValue;
    bool has_measurementTime;
    uint32_t measurementTime;
} GpsMessage;

typedef struct _SensorDataMessage {
    CommonHeader header;
    pb_size_t litersPerHour_count;
    uint32_t litersPerHour[24];
    pb_size_t tempPerHour_count;
    uint32_t tempPerHour[24];
    pb_size_t humidityPerHour_count;
    uint32_t humidityPerHour[24];
    pb_size_t strokesPerHour_count;
    uint32_t strokesPerHour[24];
    pb_size_t strokeHeightPerHour_count;
    uint32_t strokeHeightPerHour[24];
    bool has_dailyLiters;
    uint32_t dailyLiters;
    bool has_avgLiters;
    uint32_t avgLiters;
    bool has_totalLiters;
    uint32_t totalLiters;
    bool has_breakdown;
    bool breakdown;
    bool has_pumpCapacity;
    uint32_t pumpCapacity;
    bool has_pumpUsage;
    uint32_t pumpUsage;
    bool has_dryStrokes;
    uint32_t dryStrokes;
    bool has_dryStrokeHeight;
    uint32_t dryStrokeHeight;
    bool has_pumpUnusedTime;
    uint32_t pumpUnusedTime;
} SensorDataMessage;

typedef struct _StatusMessage {
    CommonHeader header;
} StatusMessage;


/* Helper constants for enums */
#define _eState_MIN eState_ACTIVATED
#define _eState_MAX eState_FAULT
#define _eState_ARRAYSIZE ((eState)(eState_FAULT+1))


/* Initializer values for message structs */
#define CommonHeader_init_default                {0, 0, 0, 0, 0, 0, false, 0, false, 0, false, _eState_MIN, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, "", false, 0, false, 0, false, 0, 0}
#define StatusMessage_init_default               {CommonHeader_init_default}
#define GpsMessage_init_default                  {CommonHeader_init_default, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define SensorDataMessage_init_default           {CommonHeader_init_default, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define CommonHeader_init_zero                   {0, 0, 0, 0, 0, 0, false, 0, false, 0, false, _eState_MIN, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, "", false, 0, false, 0, false, 0, 0}
#define StatusMessage_init_zero                  {CommonHeader_init_zero}
#define GpsMessage_init_zero                     {CommonHeader_init_zero, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}
#define SensorDataMessage_init_zero              {CommonHeader_init_zero, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define CommonHeader_productId_tag               1
#define CommonHeader_timestamp_tag               2
#define CommonHeader_msgNumber_tag               3
#define CommonHeader_fwMajor_tag                 4
#define CommonHeader_fwMinor_tag                 5
#define CommonHeader_fwBuild_tag                 6
#define CommonHeader_voltage_tag                 7
#define CommonHeader_powerRemaining_tag          8
#define CommonHeader_state_tag                   9
#define CommonHeader_activatedDate_tag           10
#define CommonHeader_magnetDetected_tag          11
#define CommonHeader_errorBits_tag               12
#define CommonHeader_numSSMResets_tag            13
#define CommonHeader_lastSSMResetDate_tag        14
#define CommonHeader_numAMResets_tag             15
#define CommonHeader_lastAMResetDate_tag         16
#define CommonHeader_logs_tag                    17
#define CommonHeader_rssi_tag                    18
#define CommonHeader_connectTime_tag             19
#define CommonHeader_mfgComplete_tag             20
#define CommonHeader_imei_tag                    21
#define GpsMessage_header_tag                    1
#define GpsMessage_hours_tag                     2
#define GpsMessage_minutes_tag                   3
#define GpsMessage_latitude_tag                  4
#define GpsMessage_longitude_tag                 5
#define GpsMessage_altitude_tag                  6
#define GpsMessage_fixQuality_tag                7
#define GpsMessage_satellitesTracked_tag         8
#define GpsMessage_hdopValue_tag                 9
#define GpsMessage_measurementTime_tag           10
#define SensorDataMessage_header_tag             1
#define SensorDataMessage_litersPerHour_tag      2
#define SensorDataMessage_tempPerHour_tag        3
#define SensorDataMessage_humidityPerHour_tag    4
#define SensorDataMessage_strokesPerHour_tag     5
#define SensorDataMessage_strokeHeightPerHour_tag 6
#define SensorDataMessage_dailyLiters_tag        7
#define SensorDataMessage_avgLiters_tag          8
#define SensorDataMessage_totalLiters_tag        9
#define SensorDataMessage_breakdown_tag          10
#define SensorDataMessage_pumpCapacity_tag       11
#define SensorDataMessage_pumpUsage_tag          12
#define SensorDataMessage_dryStrokes_tag         13
#define SensorDataMessage_dryStrokeHeight_tag    14
#define SensorDataMessage_pumpUnusedTime_tag     15
#define StatusMessage_header_tag                 1

/* Struct field encoding specification for nanopb */
#define CommonHeader_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, UINT32,   productId,         1) \
X(a, STATIC,   REQUIRED, UINT32,   timestamp,         2) \
X(a, STATIC,   REQUIRED, UINT32,   msgNumber,         3) \
X(a, STATIC,   REQUIRED, UINT32,   fwMajor,           4) \
X(a, STATIC,   REQUIRED, UINT32,   fwMinor,           5) \
X(a, STATIC,   REQUIRED, UINT32,   fwBuild,           6) \
X(a, STATIC,   OPTIONAL, UINT32,   voltage,           7) \
X(a, STATIC,   OPTIONAL, UINT32,   powerRemaining,    8) \
X(a, STATIC,   OPTIONAL, UENUM,    state,             9) \
X(a, STATIC,   OPTIONAL, UINT32,   activatedDate,    10) \
X(a, STATIC,   OPTIONAL, BOOL,     magnetDetected,   11) \
X(a, STATIC,   OPTIONAL, UINT32,   errorBits,        12) \
X(a, STATIC,   OPTIONAL, UINT32,   numSSMResets,     13) \
X(a, STATIC,   OPTIONAL, UINT32,   lastSSMResetDate,  14) \
X(a, STATIC,   OPTIONAL, UINT32,   numAMResets,      15) \
X(a, STATIC,   OPTIONAL, UINT32,   lastAMResetDate,  16) \
X(a, STATIC,   OPTIONAL, STRING,   logs,             17) \
X(a, STATIC,   OPTIONAL, UINT32,   rssi,             18) \
X(a, STATIC,   OPTIONAL, UINT32,   connectTime,      19) \
X(a, STATIC,   OPTIONAL, BOOL,     mfgComplete,      20) \
X(a, STATIC,   REQUIRED, UINT64,   imei,             21)
#define CommonHeader_CALLBACK NULL
#define CommonHeader_DEFAULT NULL

#define StatusMessage_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  header,            1)
#define StatusMessage_CALLBACK NULL
#define StatusMessage_DEFAULT NULL
#define StatusMessage_header_MSGTYPE CommonHeader

#define GpsMessage_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  header,            1) \
X(a, STATIC,   OPTIONAL, UINT32,   hours,             2) \
X(a, STATIC,   OPTIONAL, UINT32,   minutes,           3) \
X(a, STATIC,   OPTIONAL, FLOAT,    latitude,          4) \
X(a, STATIC,   OPTIONAL, FLOAT,    longitude,         5) \
X(a, STATIC,   OPTIONAL, FLOAT,    altitude,          6) \
X(a, STATIC,   OPTIONAL, UINT32,   fixQuality,        7) \
X(a, STATIC,   OPTIONAL, UINT32,   satellitesTracked,   8) \
X(a, STATIC,   OPTIONAL, FLOAT,    hdopValue,         9) \
X(a, STATIC,   OPTIONAL, UINT32,   measurementTime,  10)
#define GpsMessage_CALLBACK NULL
#define GpsMessage_DEFAULT NULL
#define GpsMessage_header_MSGTYPE CommonHeader

#define SensorDataMessage_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  header,            1) \
X(a, STATIC,   REPEATED, UINT32,   litersPerHour,     2) \
X(a, STATIC,   REPEATED, UINT32,   tempPerHour,       3) \
X(a, STATIC,   REPEATED, UINT32,   humidityPerHour,   4) \
X(a, STATIC,   REPEATED, UINT32,   strokesPerHour,    5) \
X(a, STATIC,   REPEATED, UINT32,   strokeHeightPerHour,   6) \
X(a, STATIC,   OPTIONAL, UINT32,   dailyLiters,       7) \
X(a, STATIC,   OPTIONAL, UINT32,   avgLiters,         8) \
X(a, STATIC,   OPTIONAL, UINT32,   totalLiters,       9) \
X(a, STATIC,   OPTIONAL, BOOL,     breakdown,        10) \
X(a, STATIC,   OPTIONAL, UINT32,   pumpCapacity,     11) \
X(a, STATIC,   OPTIONAL, UINT32,   pumpUsage,        12) \
X(a, STATIC,   OPTIONAL, UINT32,   dryStrokes,       13) \
X(a, STATIC,   OPTIONAL, UINT32,   dryStrokeHeight,  14) \
X(a, STATIC,   OPTIONAL, UINT32,   pumpUnusedTime,   15)
#define SensorDataMessage_CALLBACK NULL
#define SensorDataMessage_DEFAULT NULL
#define SensorDataMessage_header_MSGTYPE CommonHeader

extern const pb_msgdesc_t CommonHeader_msg;
extern const pb_msgdesc_t StatusMessage_msg;
extern const pb_msgdesc_t GpsMessage_msg;
extern const pb_msgdesc_t SensorDataMessage_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define CommonHeader_fields &CommonHeader_msg
#define StatusMessage_fields &StatusMessage_msg
#define GpsMessage_fields &GpsMessage_msg
#define SensorDataMessage_fields &SensorDataMessage_msg

/* Maximum encoded size of messages (where known) */
#define CommonHeader_size                        220
#define StatusMessage_size                       223
#define GpsMessage_size                          273
#define SensorDataMessage_size                   993

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif