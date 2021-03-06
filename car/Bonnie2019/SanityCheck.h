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

// Check if Accidantially more than one Board is active at a time
#ifdef BOARD_DASHBOARD
    #ifdef BOARD_MASTER
        #error "Two or more Boards are selected at the same Time. Check platformio.ini"
    #endif
    #ifdef BOARD_PEDAL
        #error "Two or more Boards are selected at the same Time. Check platformio.ini"
    #endif
#endif

#ifdef BOARD_PEDAL
    #ifdef BOARD_MASTER
        #error "Two or more Boards are selected at the same Time. Check platformio.ini"
    #endif
    #ifdef BOARD_DASHBOARD
        #error "Two or more Boards are selected at the same Time. Check platformio.ini"
    #endif
#endif

// Check if Framework mBed and Arduino is active at the same time
#ifdef USE_MBED
    #ifdef USE_ARDUINO
        #error "More than one Framework is currently active. Check platformio.ini"
    #endif
#endif