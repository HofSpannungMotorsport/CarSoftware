#ifndef SD_CARD_TEST_H
#define SD_CARD_TEST_H

#include "SDFileSystem.h"
#include "../../Bonnie2019/hardware/Pins_Master.h"

SDFileSystem sd(MASTER_PIN_SPI_SD_MOSI, MASTER_PIN_SPI_SD_MISO, MASTER_PIN_SPI_SD_SCK, MASTER_PIN_SPI_SD_CS, "sd");

void SDCardTest() {
    wait(3);
    printf("Hello World!\n");

    sd.disk_initialize();
    if (sd.disk_status() == 0) {
        printf("Disk OK\n");
    } else {
        printf("Disk Error\n");
    }
 
    mkdir("/sd/mydir", 0777);
    
    {
        FILE *fp = fopen("/sd/mydir/sdtest.txt", "w");
        if(fp == NULL) {
            error("Could not open file for write\n");
        } else {
            fprintf(fp, "Hello fun SD Card World!\n");
            fprintf(fp, "Hello fun SD Card World!\n");
        }
        
        fclose(fp);
    }

    {
        FILE *fp = fopen("/sd/mydir/sdtest.txt", "a");
        if(fp == NULL) {
            error("Could not open file for write\n");
        } else {
            fprintf(fp, "Hello fun SD Card World!\n");
            fprintf(fp, "Hello fun SD Card World!\n");
            fprintf(fp, "Hello fun SD Card World!\n");
        }

        fclose(fp);
    }
 
    printf("Goodbye World!\n");

    wait(3);
}

#endif // SD_CARD_TEST_H