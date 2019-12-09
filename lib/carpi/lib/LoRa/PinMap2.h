/*
 * Copyright (c) 2018 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */



#ifdef TARGET_NUCLEO_L476RG
 #define FEATURE_LORA
 
#elif TARGET_DISCO_L072CZ_LRWAN1
 #define FEATURE_LORA

#elif TARGET_STM32L432KC
 #define HELTEC_STM32L4
 #define FEATURE_LORA
#endif



#if defined(TARGET_DISCO_L072CZ_LRWAN1)
#define LED             LED2

#define LORA_SPI_MOSI   PA_7
#define LORA_SPI_MISO   PA_6
#define LORA_SPI_SCLK   PB_3
#define LORA_CS         PA_15
#define LORA_RESET      PC_0
#define LORA_DIO0       PB_4
#define LORA_DIO1       PB_1
#define LORA_DIO2       PB_0
#define LORA_DIO3       PC_13
#define LORA_DIO4       PA_5
#define LORA_DIO5       PA_4
#define LORA_ANT_RX     PA_1
#define LORA_ANT_TX     PC_2
#define LORA_ANT_BOOST  PC_1
#define LORA_TCXO       PA_12   // 32 MHz



#elif defined(TARGET_NUCLEO_L476RG) // using the RFM95 board

#define LORA_SPI_MOSI   PC_12
#define LORA_SPI_MISO   PC_11
#define LORA_SPI_SCLK   PC_10
#define LORA_CS         PA_0
#define LORA_RESET      PA_1
#define LORA_DIO0       PD_2    // DIO0=TxDone/RXDone
#define LORA_DIO1       PB_7    //
#define LORA_DIO2       PC_14   // DIO2=FhssChangeChannel
#define LORA_DIO3       PC_15   // DIO3=CADDone
#define LORA_DIO4       PH_0    // ????
#define LORA_DIO5       NC      // unused?

#elif defined (HELTEC_STM32L4)

#define USER_BUTTON     PH_3    // boot pin
#define LED             PB_1    // green
#define LED1            LED
#define LED2            PB_0    // red

#define POWER_VEXT      PA_3
#define POWER_VEXT_ON   0
#define POWER_VEXT_OFF  1


#define LORA_SPI_MOSI   PA_7
#define LORA_SPI_MISO   PA_6
#define LORA_SPI_SCLK   PA_5
#define LORA_CS         PA_4
#define LORA_RESET      PA_1
#define LORA_DIO0       PA_0    // DIO0=TxDone/RXDone/CADDone
#define LORA_DIO1       NC      //
#define LORA_DIO2       NC      // 
#define LORA_DIO3       NC      // 
#define LORA_DIO4       NC      // 
#define LORA_DIO5       NC      // 

#elif defined(TARGET_NUCLEO_L432KC) // using the RFM95 board

// #define LED             PB_3    // green
#define LED             PB_5    // green

#define LORA_SPI_MOSI   PA_7
#define LORA_SPI_MISO   PA_6
#define LORA_SPI_SCLK   PB_5
#define LORA_CS         PA_4
#define LORA_RESET      PA_1
#define LORA_DIO0       PA_0    // DIO0=TxDone/RXDone/CADDone
#define LORA_DIO1       NC      //
#define LORA_DIO2       NC      // 
#define LORA_DIO3       NC      // 
#define LORA_DIO4       NC      // 
#define LORA_DIO5       NC      //


#else 

#error "unknown board"

#endif