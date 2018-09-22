#!/bin/bash

# compile kernel module
make clean
make

# if compilation has produced a loadable kernel module
if [ -f kernelSrc/ipclearn.ko ]
then
	# if the kernel module is already installed  
	lsmod | grep ipclearn >& /dev/null
	if [ $? -eq 0 ]
	then
		# uninstall the existing kernel module  
    	sudo rmmod ipclearn 
	fi

	# install the new version of the kernel module
	sudo insmod kernelSrc/ipclearn.ko

	# remove user's executable if it exists
	[ -x process1/process1.c ] && rm process1/process1
	
	# compile and execute the user code which uses the module
	gcc process1/process1.c -o process1/process1
	
	./process1/process1
fi
