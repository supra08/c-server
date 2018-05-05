
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#define MAXLIMIT 1000
#define BUFFER 1024

char *ROOT;
int listenfd, clients[MAXLIMIT];
//void error(char *);
void respond(int);

int main(int argc, char* argv[])
{
	struct sockaddr_in clientaddr;
	struct addrinfo hints, *res, *p;
	socklen_t addrlen;
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10000");

	int id=0;//represents ids for each client
	
	printf("Server started at port no. %s%s%s\n","\033[92m",PORT,"\033[0m");

	//______________________________________________
	//initializing the clients with default values i.e. not connected
	for (int i=0; i<MAXLIMIT; i++)
		clients[i]=-1;
	//______________________________________________

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo( NULL, PORT, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket(p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) 
			continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) 
			break;
	}


	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 1000) != 0 )
	{
		perror("listen() error, too many connections");
		exit(1);
	}
	//____________________________________________________________________________

	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[id] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
			if ( fork()==0 )
			{
				respond(id);
				exit(0);
			}
		while (clients[id]!=-1) id = (id+1)%MAXLIMIT;
	}

	return 0;
}



void respond(int n)
{
	char mesg[99999], *header[2], datagram[BUFFER], path[99999];
	int received_msg, fd, bytes_read;

	memset( (void*)mesg, (int)'\0', 99999 );

	received_msg=recv(clients[n], mesg, 99999, 0);

	if (received_msg<0)    // receive error
		fprintf(stderr,("receive error\n"));
	else    // message received
	{
		printf("%s", mesg);
		header[0] = strtok (mesg, " \t\n");
		if ( strncmp(header[0], "GET\0", 4)==0 )
		{
			header[1] = strtok (NULL, " \t");
				if ( strncmp(header[1], "/\0", 2)==0 )
					header[1] = "/html/index.html";      

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], header[1]);
				printf("file: %s\n", path);

				//reading from the actual file
				if ( (fd=open(path, O_RDONLY))!=-1 )  
				{
					send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
					while ( (bytes_read=read(fd, datagram, BUFFER))>0 )
						write (clients[n], datagram, bytes_read);
				}
				else    write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
			
		}
	}

	//____________________________________________________
	//shutting down the server when the work is complete 
	shutdown (clients[n], SHUT_RDWR);      
	close(clients[n]);
	clients[n]=-1;
}


