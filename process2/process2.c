#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>

/* CONSTANTS */
#define DEVICE_FILE "/dev/binder_test_ipc"
#define LENGTH      1024
#define READ        'r'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE


/******************************************************************************
 Entry point of the process.
******************************************************************************/
int main()
{
    int fd, done = FALSE;
    char  read_buf[LENGTH] ,command[LENGTH/8];
    
    struct data
    {
        int src;
        int dest;
        int destLength;
        char buf[LENGTH];
    };


    /* Give permission to use the device file */
    sprintf(command, "sudo chmod 777 %s", DEVICE_FILE);
    system(command);

    assert(fd = open(DEVICE_FILE, O_RDWR));
    do{
 
            read(fd, read_buf, LENGTH);
	    struct data* myData = (struct data*) read_buf; 
            printf("device: %d %d %d %s\n", myData->src, myData->dest,myData->destLength, myData->buf);
	 sleep(2);
    }while(1);
    close(fd);
    return 0;
}
