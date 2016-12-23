#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h> //sockaddr_in
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define BUFSIZE 1024

void* clnt_connection(void* arg);
void send_message(char * message , int len);

int clnt_number = 0;
int clnt_socks[10];
pthread_mutex_t mutx; //mutex var

int main(int argc , char * argv[])
{
	int serv_sock;
	int clnt_sock;	//client socket description
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	pthread_t thread;

	if(argc != 2)
	{
		printf("argc error");
		exit(1);
	}
	
	if(pthread_mutex_init(&mutx,NULL)) //mutex initializing , 첫번째 인자 : 초기화할 뮤텍스포인터/두번째 인자 : 뮤텍스의 특성
	{
		printf(strerror(errno));
		exit(0);
	}

	serv_sock = socket(AF_INET, SOCK_STREAM , 0);  //socket description
	
	memset(&serv_addr , 0 , sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1])); //port

	if(bind(serv_sock , (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)	
	{
		printf(strerror(errno));
		exit(0);
	}

	if(listen(serv_sock,5) == -1)	
	{
		printf(strerror(errno));
		exit(0);
	}
	while(1)
	{
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock , (struct sockaddr*)&clnt_addr ,&clnt_addr_size);
		
		pthread_mutex_lock(&mutx); // 다른 쓰레드가 접근할 수 없도록lock
		clnt_socks[clnt_number++] = clnt_sock; //임계영역 
		pthread_mutex_unlock(&mutx);// mutex unlock

		pthread_create(&thread,NULL,clnt_connection, (void*)clnt_sock);
		printf("new connection ip : %s\n", inet_ntoa(clnt_addr.sin_addr));
	}
	return 0;
}

void *clnt_connection(void *arg)
{
	int clnt_sock = (int)arg;
	int str_len = 0;
	char message[BUFSIZE];
	int i;
	
	while( (str_len = read(clnt_sock ,message , sizeof(message))) != 0 )
		send_message(message, str_len);
	
	/* client exit*/
	pthread_mutex_lock(&mutx);
	for(i = 0 ; i<clnt_number ; i++)
	{
		if(clnt_sock == clnt_socks[i])
		{
			for(;i<clnt_number-1;i++)
				clnt_socks[i] = clnt_socks[i+1];
			printf("disconnected\n");
			break;
		}
	}
	clnt_number--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return 0;
}

void send_message(char * message, int len)
{
	int i ;
	pthread_mutex_lock(&mutx);
	for(i=0;i<clnt_number ; i++)
		write(clnt_socks[i],message ,len);
	pthread_mutex_unlock(&mutx);
}

