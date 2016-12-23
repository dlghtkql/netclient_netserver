#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> 	// sockaddr_in
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define BUFSIZE 1024
// socket -> connect 

int main(int argc, char *argv[])
{
	int sock;
	char message[BUFSIZE];
	int str_len;
	struct sockaddr_in serv_addr;

	if(argc != 3)
	{
		printf("argc error");
		exit(1);
	}

	sock = socket(AF_INET,SOCK_STREAM , 0); //socket description
	if(sock == -1)
	{
		printf(strerror(errno));
		exit(0);
	}

	memset(&serv_addr , 0 , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //ip
	serv_addr.sin_port = htons(atoi(argv[2]));	//port

	if( connect(sock , (struct sockaddr*)&serv_addr , sizeof(serv_addr)) == -1 )
	{
		printf(strerror(errno));
		exit(0);
	}

	while(1)
	{
		fputs("input message (quit to quit) : ",stdout);
		fgets(message,BUFSIZE,stdin);

		write(sock , message, strlen(message));
		if(!strcmp(message,"quit\n"))
			break;
	//	write(sock , message, strlen(message));
		
		str_len = read(sock , message , BUFSIZE-1);
		message[str_len] = 0;
		printf("message(from the server) : %s\n",message);
		memset(message,0,sizeof(message));
	}
	close(sock);
	return 0;
}

