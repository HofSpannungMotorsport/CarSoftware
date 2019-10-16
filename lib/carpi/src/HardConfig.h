#ifndef HARDCONFIG_H
#define HARDCONFIG_H

// Components
//   Button
#define BUTTON_STATE_BUFFER_SIZE 64

//   Pedal
#define PEDAL_CALIBRATION_SAMPLE_BUFFER_SIZE  20 // How many values should be combined during calibration to get the fu***** deviance away

//   RPM Sensor
#define RPM_MEASUREMENT_TIMEOUT 0.5 // s -> 500ms
#define RPM_MEASUREMENT_POINTS_PER_REVOLUTION 12
// #define RPM_USE_FALL // STD it uses the rising edge. Decomment to use falling edge as measurement point
#define RPM_AVERAGE_OVER_MEASUREMENT_POINTS 2 // Over how many measurement points should the real speed be calculated? HAS TO BE AT LEAST 1

//   SD Card
#define SD_CARD_ROOT "sd"
#define SD_CARD_TRANSFER_SPEED 16000000 // -> 16 MHz
#define SD_LOG_FOLDER_PATH "/sd/log"
#define SD_LOG_FILE_PATH "/sd/log/log.csv"

//   HV Enable
#define STD_HV_ENABLED_DEBOUNCE_TIME 0.1 // s


// Programs
//   PLogger
#define STD_LOGGER_REFRESH_RATE 1
#define STD_LOGGER_FILE_BEGIN_MESSAGE "Begin_Logging"


// Services
//   SCar
#define SCAR_ERROR_REGISTER_SIZE 64 // errors, max: 255


// Communication
//   Sync
#define STD_SYNC_INCOMING_MESSAGES_BUFFER_SIZE 128 // max 255
//   CCAN
#define STD_CCAN_FREQUENCY 250000
#define STD_CCAN_MAX_OUT_QUEUE_SIZE 128 // Elements
#define STD_CCAN_OUT_QUEUE_IMPORTANT_THRESHHOLD STD_CCAN_MAX_OUT_QUEUE_SIZE/2
#define STD_CCAN_TIMEOUT (1.0/(float)STD_CCAN_FREQUENCY) // s

#endif // HARDCONFIG_H