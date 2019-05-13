#ifndef SD_CARD_TEST_H
#define SD_CARD_TEST_H

#include "SD_Lib/SDFileSystem.h"

#define SPI_CS   D9
#define SPI_SCK  D13
#define SPI_MOSI D11
#define SPI_MISO D12

SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS, "sd");

void SDCardTest() {
    printf("Hello World!\n");   
 
    mkdir("/sd/mydir", 0777);
    
    {
        FILE *fp = fopen("/sd/mydir/sdtest.txt", "w");
        if(fp == NULL) {
            error("Could not open file for write\n");
        }
        fprintf(fp, "Hello fun SD Card World!\n");
        fprintf(fp, "Hello fun SD Card World!\n");
        fclose(fp);
    }

    {
        FILE *fp = fopen("/sd/mydir/sdtest.txt", "a");
        if(fp == NULL) {
            error("Could not open file for write\n");
        }
        fprintf(fp, "Hello fun SD Card World!\n");
        fprintf(fp, "Hello fun SD Card World!\n");
        fprintf(fp, "Hello fun SD Card World!\n");
        fclose(fp);
    }
 
    printf("Goodbye World!\n");

    while(true);
}

#endif // SD_CARD_TEST_H