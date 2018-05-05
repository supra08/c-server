 # build an executable named cserver from cserver.c
 all:cserver.c 
	gcc -g -Wall -o cserver cserver.c

 clean: 
	rm cserver