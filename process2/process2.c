#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

/* CONSTANTS */
#define DEVICE_FILE "/dev/binder_test_ipc"
#define LENGTH      1024
#define READ        'r'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE
#define PAGE_SIZE 1024 

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
    char buf[PAGE_SIZE];
    char * address = NULL;
    assert(fd = open(DEVICE_FILE, O_RDWR));
    address = mmap(NULL, 1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) {
	    perror("mmap");
	    return -1;
    }
    FILE *picfd = fopen("revised_1mb.txt", "w");
    if(picfd == NULL)
	    return -1;

    do{
	    fwrite(address, sizeof(char), PAGE_SIZE*1024, picfd);
	    //printf("data: %s\n", address);
	    sleep(2);
    }while(0);
    close(fd);
    fclose(picfd);
    getchar();
    return 0;
}
