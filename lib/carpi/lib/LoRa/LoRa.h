#ifndef __LoRa
#define __LoRa

#include "mbed.h"
#include "PinMap2.h"
#include "sx1276-mbed-hal.h"

#ifdef FEATURE_LORA

/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1

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


#define RX_TIMEOUT_VALUE    3500	// in ms

//#define BUFFER_SIZE       32        // Define the payload size here
#define BUFFER_SIZE         64        // Define the payload size here

Serial pcSerial(USBTX, USBRX);


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

volatile AppStates_t State = LOWPOWER;

/*!
* Radio events function pointer
*/
static RadioEvents_t RadioEvents;

/*
*  Global variables declarations
*/
SX1276Generic *Radio;


DigitalOut *led3;

void OnTxDone(void *radio, void *userThisPtr, void *userData);
void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void *radio, void *userThisPtr, void *userData);
void OnRxTimeout(void *radio, void *userThisPtr, void *userData);
void OnRxError(void *radio, void *userThisPtr, void *userData);


class LoRa: public IChannel {
private:
    Sync &_syncer;

    uint8_t Message[]; //TODO: fill with carMessage

    uint16_t BufferSize = BUFFER_SIZE;
    uint8_t *Buffer;

public:
    LoRa(Sync &syncer) : _syncer(syncer) {}

    void send(CarMessage &carMessage)()
    {
        //TODO: fill with parsed carMessage
        Message = carMessage;

        DigitalOut *led = new DigitalOut(LED4);   // RX red
        led3 = new DigitalOut(LED3);  // TX blue
        
        Buffer = new uint8_t[BUFFER_SIZE];
        *led3 = 1;

        Radio = new SX1276Generic(NULL, RFM95_SX1276,
                LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
                LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
        
        uint8_t i;

        // Initialize Radio driver
        RadioEvents.TxDone = OnTxDone;
        RadioEvents.RxDone = OnRxDone;
        RadioEvents.RxError = OnRxError;
        RadioEvents.TxTimeout = OnTxTimeout;
        RadioEvents.RxTimeout = OnRxTimeout;
        if (Radio->Init( &RadioEvents ) == false) {
            if (DEBUG_MESSAGE) {
                while(1) {
                    printf("Radio could not be detected!");
                    wait_us( 1000000 ); // 1s
                }
            }
        }

        Radio->SetChannel(RF_FREQUENCY);

        Radio->SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                            LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                            LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                            LORA_IQ_INVERSION_ON, 2000 );

        Radio->SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                            LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                            LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                            LORA_IQ_INVERSION_ON, true );


        //Radio->Rx( RX_TIMEOUT_VALUE );
        State = RX;
        
        while( 1 )
        {
            switch( State )
            {
            case RX:    // reading happend
                printf("RX");
                *led3 = 0;
                if( BufferSize > 0 )
                {
                    *led = !*led;
                    memcpy(Buffer, Message, sizeof(Message));
                    // We fill the buffer with numbers for the payload 
                    for( i = sizeof(Message); i < BufferSize; i++ )
                    {
                        Buffer[i] = i - sizeof(Message);
                    }
                    wait_us( 10000 );   // 10ms
                    Radio->Send( Buffer, BufferSize );
                }
                State = LOWPOWER;
                break;
            case TX:    // sending happend
                *led3 = 1;
                Radio->Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
                break;
            case RX_TIMEOUT:
                State = RX;
                break;
            case RX_ERROR:  // reading error
                *led = !*led;
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
};


void OnTxDone(void *radio, void *userThisPtr, void *userData)
{
    Radio->Sleep( );
    State = TX;
    if (DEBUG_MESSAGE)
        printf("> OnTxDone\n");
}

void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio->Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    State = RX;
    if (DEBUG_MESSAGE)
        printf("> OnRxDone: RssiValue=%d dBm, SnrValue=%d \n", rssi, snr);
}

void OnTxTimeout(void *radio, void *userThisPtr, void *userData)
{
    *led3 = 0;
    Radio->Sleep( );
    State = TX_TIMEOUT;
    if(DEBUG_MESSAGE)
        printf("> OnTxTimeout\n");
}

void OnRxTimeout(void *radio, void *userThisPtr, void *userData)
{
    *led3 = 0;
    Radio->Sleep( );
    Buffer[BufferSize-1] = 0;
    State = RX_TIMEOUT;
    if (DEBUG_MESSAGE)
        printf("> OnRxTimeout\n");
}

void OnRxError(void *radio, void *userThisPtr, void *userData)
{
    Radio->Sleep( );
    State = RX_ERROR;
    if (DEBUG_MESSAGE)
        printf("> OnRxError\n");
}

#endif
#endif