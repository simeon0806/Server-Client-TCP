#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PORT (8080)
#define MSG_SIZE (64)
#define NAME_LEN (32)
#define END_MSG ("exit")
#define WORKERS_NUM (6)

pthread_mutex_t mutex;
pthread_cond_t cond;
int cash = 500;

void* workerHandler(void* arg);

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("You should enter one argument\n->Time for simulating the Shop\n");
		exit(-1);
	}

	int workTime = atoi(argv[1]);
	printf("\nThe store will works for %d seconds\n", workTime);
	printf("------------------------------------------------------------------------\n\n");


	int sockfd, connfd[WORKERS_NUM], len;
	struct sockaddr_in servaddr, cli;
	pthread_t thread[WORKERS_NUM];

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(1);
	}

	bzero((char*)&servaddr, sizeof(servaddr));

	// Assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);


	// Binding created socket to given IP
	if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
		printf("socket bind failed...\n");
		exit(1);
	}

	// Now server is ready to listen
	if ((listen(sockfd, WORKERS_NUM)) != 0) {
		printf("Listen failed...\n");
		exit(1);
	}

	len = sizeof(cli);

	int i=0;
	for(i=0;i<WORKERS_NUM;i++){
		// Accept the data packet from client
		connfd[i] = accept(sockfd, (struct sockaddr*)&cli, &len);
		// Check if the client is accepted
		if (connfd[i] < 0) {
			printf("server acccept failed...\n");
			exit(1);
		}

		// Creating thread for every new client
		if(pthread_create(&thread[i],NULL,workerHandler,(void*)&connfd[i])!=0)
		{
			printf("pthread_create failed...\n");
			exit(1);
		}
	}

	// Sleep the main thread
	sleep(workTime);

	for(i=0;i<WORKERS_NUM;i++){
		write(connfd[i], END_MSG, 5);
	}
	
	for(i=0;i<WORKERS_NUM;i++)
	{
		pthread_cancel(thread[i]);
	}

	close(sockfd);

	printf("------------------------------------------------------------------------\n");
	printf("The store is closed\n\n");

	return 0;
}

void* workerHandler(void* arg)
{
	int * sockfd = (int*)arg;
	char buff[MSG_SIZE];
	int n;
	int value;
	char worker[20];

	while(1) {
		memset(buff,0,MSG_SIZE);

		// Read the message from client and copy it in buffer
		read(*sockfd, buff, sizeof(buff));
		
		// Clear the worker variable
		memset(worker, 0, NAME_LEN);

		// Copy the value
		value = atoi(buff + NAME_LEN);
		
		// Copy the worker's name
		strncpy(worker, buff, NAME_LEN);

		pthread_mutex_lock(&mutex);
		while(value+cash<0)
		{
			pthread_cond_wait(&cond,&mutex);
		}

		cash = cash + value;

		if(value>0)
		{
			printf("%s returns %d BGN. Now there is %d BGN in the cash\n", worker, value, cash);
		}
		else
		{
			printf("%s takes   %d BGN. Now there is %d BGN in the cash\n", worker, (-1)*value, cash);
		}

		if(value>0)
		{
			pthread_cond_signal(&cond);
		}

		pthread_mutex_unlock(&mutex);

		// Clear the buffer
		memset(buff,0,MSG_SIZE);
		strcpy(buff,"reciverd");

		// Send that buffer to client
		write(*sockfd, buff, sizeof(buff));
	}
}
