#ifndef SANITYCHECK_H
#define SANITYCHECK_H


// Check if the right board is selected before compiling (to avoid compiling errors and compiling for the wrong board)
#ifdef BOARD_DASHBOARD
    #ifndef STM32F446xx
        #error "Wrong Board choosen! Select Nucleo F446RE as your desiered Board (Dashboard Controller). Check platformio.ini"
    #endif
#endif

#ifdef BOARD_PEDAL
    #ifndef STM32F446xx
        #error "Wrong Board choosen! Select Nucleo F446RE as your desiered Board (Pedal Controller). Check platformio.ini"
    #endif
#endif

#ifdef BOARD_MASTER
    #ifndef STM32F767xx
        #error "Wrong Board choosen! Select Nucleo F767ZI as your desiered Board (Master Controller/LV-Box). Check platformio.ini"
    #endif
#endif

#endif // SANITYCHECK_H