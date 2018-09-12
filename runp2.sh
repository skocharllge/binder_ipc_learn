#!/bin/bash

# compile kernel module
make clean
make

# if compilation has produced a loadable kernel module
if [ -f kernelSrc/ipclearn.ko ]
then
	# remove user's executable if it exists
	[ -x process2/process2 ] && rm process2/process2
	
	# compile and execute the user code which uses the module
	gcc process2/process2.c -o process2/process2
	./process2/process2
fi
