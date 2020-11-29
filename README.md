# Device-Driver
cs453 p3


## Overview
write a simple character driver called booga. 
Note that, you will only be able to test this assignment
on a Linux machine where you have root privilege. 
A VMware-based CentOS 7 (64 bit) VM image is provided. 
Later on we will refer to this VM as the cs453 VM. 
(username/password: cs453/cs453, run commands with sudo 
to gain root privilege). You can also download a CentOS 7 (64 bit)
and install it by yourself, and you can also use VirtualBox.

## Build & run

To build:

	make 
	sudo ./booga_load

## Features and usage

To run
	./test-booga <minor number> <buffersize> <read|write>
	
	Example: ./test-booga 0 100 read
			 ./test-booga 0 100 write
	
	sudo ./booga_unload

Using valgrind
	make valgrind minor=<minor number> buffer=<buffer number> rw=<read or write>

## Testing & Valgrind

[cs453@localhost testp3]$ ./test-booga 0 100 read
 booga returned 100 characters
googoo! gaagaa!googoo! gaagaa!googoo! gaagaa!googoo! gaagaa!googoo! gaagaa!googoo! gaagaa!googoo! ga

[cs453@localhost testp3]$ ./test-booga 0 100 write
Attempting to write to booga device
Wrote 100 bytes.

[cs453@localhost testp3]$ cat /proc/driver/booga
bytes read = 600
bytes written = 300
number of opens:
/dev/booga0 = 9 times
/dev/booga1 = 0 times
/dev/booga2 = 0 times
/dev/booga3 = 3 times
strings output:
booga! booga! = 3 times
googoo! gaagaa! = 1 times
wooga! wooga! = 0 times
neka! maka! = 2 times
