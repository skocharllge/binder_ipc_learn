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
#define APPBUF      'a'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE

int main()
{
    int fd, done = FALSE;

    struct data
    {
	int src;
	int dest;
	int destLength;
	char buf[LENGTH];
    };
    
    struct data myBuf;
    char command[LENGTH/8];
    sprintf(command, "sudo chmod 777 %s", DEVICE_FILE);
    system(command);
    assert(fd = open(DEVICE_FILE, O_RDWR));
    
    do
    {
    	printf("writing modified buffer\n");
    	myBuf.src = 1;
    	myBuf.dest = 2;
    	myBuf.destLength = 50;
    	strcpy(myBuf.buf, "srinivas");
    	write(fd, (char*)&myBuf, sizeof(struct data));
	sleep(2);
	
    }while(1);
    close(fd);
    return 0;
}
