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
    char buf[1024*1024];
    char * address = NULL;
    assert(fd = open(DEVICE_FILE, O_RDWR));
    address = mmap(NULL, 1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) {
	    perror("mmap");
	    return -1;
    }
    long int numbytes;
    FILE *picfd = fopen("1mb.txt", "r");
    if(picfd == NULL)
	    return -1;

    do{
	    /* Get the number of bytes */
	    fseek(picfd, 0L, SEEK_END);
	    numbytes = ftell(picfd);
	    printf("num bytes: %ld\n", numbytes);
	    fseek(picfd, 0L, SEEK_SET);
	    fread(address, sizeof(char), 1024*1024, picfd);
	    //gets(buf);
	    //memcpy(address, buf, strlen(buf));

	 sleep(2);
    }while(0);
    close(fd);
    fclose(picfd);
    getchar();
    return 0;
}
