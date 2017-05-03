/******************************************************************************
 * filename: main.c
 *
 * Used to test the ethernet driver, blink the Led
 *
 * Written by: James Ross
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define LED0_ON 0x4E /* assert LED, set invery bit */
#define LED0_OFF 0xF 

// #define DEBUG 0

int main(void)
{
    int retBytes = 0;
    int fd;
    int readBuff = 0;
    int toWrite = LED0_ON;

    /* open driver */
    fd = open("/dev/pciLED", O_RDWR);
    if(fd == -1){
        printf("open error\n");
        exit(EXIT_FAILURE);
    }
    
    while(true){
        /* read led control register from driver, 32bit */
        retBytes = read(fd, &readBuff,sizeof(int)); 
        if(retBytes == -1){
            printf("read1 error\n");
            exit(EXIT_FAILURE);
        }

        /* print the returned value */
        printf("Read pre assert, result: %x\n", readBuff);
        
        /* clear first nibble */
        readBuff &= 0xFFFFFFF0;

        /* set value to write, keeping current bits */
        toWrite = readBuff | LED0_ON; /* only change desired bit */

        /* write to the led control register, make led turn on */
        retBytes = write(fd, &toWrite, sizeof(int));
        
        /* read the register again and view the result */
        retBytes = read(fd, &readBuff,sizeof(int)); 
        if(retBytes == -1){
            printf("read1 error\n");
            exit(EXIT_FAILURE);
        }
        printf("read post assert, pre-disable, result: %x\n", readBuff);

        sleep(2);

        /* read the register again and view the result */
        retBytes = read(fd, &readBuff,sizeof(int)); 
        if(retBytes == -1){
            printf("read1 error\n");
            exit(EXIT_FAILURE);
        }

        /* clear first nibble */
        readBuff &= ~LED0_ON;

        toWrite = readBuff | LED0_OFF; /* only change desired bit */

        /* write to the led control register, make led turn on */
        retBytes = write(fd, &toWrite, sizeof(int));

        sleep(2);
    }

    exit(EXIT_SUCCESS);
}
