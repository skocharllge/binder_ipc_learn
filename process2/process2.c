/******************************************************************************
                          ____     ____                                                 
                 		 / __/	  / __/__  __ _____________                            
                		/ /_	 _\ \/ _ \/ // / __/ __/ -_|                           
               			\__/	/___/\___/\___/_/  \__/\__/                            
                                                                                                                                                                          
*******************************************************************************
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
 @file: main.c
 @author: Victor Neville
 @description: Simple program which tests a character device driver. 	  			  
 @date: 13-05-2017
 *****************************************************************************/
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
