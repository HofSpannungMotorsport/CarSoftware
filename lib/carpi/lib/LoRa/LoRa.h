#ifndef __LoRa
#define __LoRa

#include "mbed.h"
#include "PinMap2.h"
#include "sx1276-mbed-hal.h"

#ifdef FEATURE_LORA

#define DEBUG_MESSAGE

#define RF_FREQUENCY            RF_FREQUENCY_868_1  // Hz
#define TX_OUTPUT_POWER         14                  // 14 dBm

#define LORA_BANDWIDTH          125000  // LoRa default, details in SX1276::BandwidthMap
#define LORA_SPREADING_FACTOR   LORA_SF7
#define LORA_CODINGRATE         LORA_ERROR_CODING_RATE_4_5

#define LORA_PREAMBLE_LENGTH    8       // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT     5       // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_FHSS_ENABLED       false  
#define LORA_NB_SYMB_HOP        4     
#define LORA_IQ_INVERSION_ON    false
#define LORA_CRC_ENABLED        true


#define RX_TIMEOUT_VALUE        3500	// in ms

#define LORA_BUFFER_SIZE        64        // Define the payload size here


/*
*  Global variables declarations
*/
typedef enum
{
    LOWPOWER = 0,
    IDLE,
    
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    
    TX,
    TX_TIMEOUT,
    
    CAD,
    CAD_DONE
} AppStates_t;

/*!
* Radio events function pointer
*/
static RadioEvents_t RadioEvents;

void OnTxDone(void *radio, void *userThisPtr, void *userData);
void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void *radio, void *userThisPtr, void *userData);
void OnRxTimeout(void *radio, void *userThisPtr, void *userData);
void OnRxError(void *radio, void *userThisPtr, void *userData);


class LoRa: public IChannel {
private:
    volatile AppStates_t State = LOWPOWER;

    Sync &_syncer;

    uint16_t BufferSize = LORA_BUFFER_SIZE;
    uint8_t Buffer[LORA_BUFFER_SIZE];

    DigitalOut ledRX;
    DigitalOut ledTX;

    SX1276Generic Radio;

public:
    LoRa(Sync &syncer) : _syncer(syncer), ledRX(LED4), ledTX(LED3),
                Radio(NULL, RFM95_SX1276,
                LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5) {

        // Initialize Radio driver
        RadioEvents.TxDone = OnTxDone;
        RadioEvents.RxDone = OnRxDone;
        RadioEvents.RxError = OnRxError;
        RadioEvents.TxTimeout = OnTxTimeout;
        RadioEvents.RxTimeout = OnRxTimeout;
        while (Radio.Init( &RadioEvents ) == false) {
            #ifdef DEBUG_MESSAGE
                printf("Radio could not be detected!");
                wait_us( 1000000 ); // 1s
            #endif
        }

        Radio.SetChannel(RF_FREQUENCY);

        Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                            LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                            LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                            LORA_IQ_INVERSION_ON, 2000 );

        Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                            LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                            LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                            LORA_IQ_INVERSION_ON, true );
    }

    void send(CarMessage &carMessage)()
    {        
        ledTX = 1;
        
        uint8_t i = 0;


        //Radio.Rx( RX_TIMEOUT_VALUE );
        State = RX;
        
        while( 1 )
        {
            switch( State )
            {
            case RX:    // reading happend
            #ifdef DEBUG_MESSAGE
                printf("RX");
            #endif
                ledTX = 0;
                if( BufferSize > 0 )
                {
                    ledRX = !ledRX;
                    memcpy(Buffer, carMessage, sizeof(carMessage));
                    // We fill the buffer with numbers for the payload 
                    for( i = sizeof(carMessage); i < BufferSize; i++ )
                    {
                        Buffer[i] = i - sizeof(carMessage);
                    }
                    wait_us( 10000 );   // 10ms
                    Radio.Send( Buffer, BufferSize );
                }
                State = LOWPOWER;
                break;
            case TX:    // sending happend
                ledTX = 1;
                //Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
                break;
            case RX_TIMEOUT:
                State = RX;
                break;
            case RX_ERROR:  // reading error
                ledRX = !ledRX;
                State = RX;
                break;
            case TX_TIMEOUT:
                State = RX;
                break;
            case LOWPOWER:
                sleep();
                break;
            default:
                State = LOWPOWER;
                break;
            }    
        }
    }

    bool messageInQueue() {
        // TODO: still to implement
        return 0;
    }
};


void OnTxDone(void *radio, void *userThisPtr, void *userData)
{
    Radio.Sleep( );
    State = TX;
    #ifdef DEBUG_MESSAGE
        printf("> OnTxDone\n");
    #endif
}

void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    State = RX;
    #ifdef DEBUG_MESSAGE
        printf("> OnRxDone: RssiValue=%d dBm, SnrValue=%d \n", rssi, snr);
    #endif
}

void OnTxTimeout(void *radio, void *userThisPtr, void *userData)
{
    ledTX = 0;
    Radio.Sleep( );
    State = TX_TIMEOUT;
    #ifdef DEBUG_MESSAGE
        printf("> OnTxTimeout\n");
    #endif
}

void OnRxTimeout(void *radio, void *userThisPtr, void *userData)
{
    ledTX = 0;
    Radio.Sleep( );
    Buffer[BufferSize-1] = 0;
    State = RX_TIMEOUT;
    #ifdef DEBUG_MESSAGE
        printf("> OnRxTimeout\n");
    #endif
}

void OnRxError(void *radio, void *userThisPtr, void *userData)
{
    Radio.Sleep( );
    State = RX_ERROR;
    #ifdef DEBUG_MESSAGE
        printf("> OnRxError\n");
    #endif
}

#endif
#endif