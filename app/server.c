#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


#include "redis.h"


void handle_request(int *arg)
{
	int fd = *(int *)arg;
	printf("Client connected: %d\n", fd);

	
	char request[255];

	while (1)
	{
		char response[256];

		printf("Receiving\n");
		int r = recv(fd, (void *)request, 255, 0);
		
		printf("Received: %s(%d) -- (%d)\n", request, strlen(request), fd);
		if (strlen(request) == 0)
		{
			printf("Ending the connection: %d\n", fd);
			break;
		}

		// Read the command 
		redis_command *command = get_redis_command(request);

		if (command->type == AGGREGATE)
		{
			if (command->child_command == NULL)
			{
				printf("Error parsing aggregate command\n");
				exit(1);
			}
			redis_command *subcommand = (redis_command *) command->child_command;

			if (strcmp(subcommand->string_command, "ECHO") == 0)
			{
				redis_command *next_command = (redis_command *) subcommand->next_command;
				build_redis_response(response, next_command->original_command);
			}
			else
			{
				strcpy(response, "+PONG\r\n");
			}
		}
		else 
		{
			strcpy(response, "+PONG\r\n");
		}

		// TODO: recursively free the memory
		free(command);

		printf("Sending: %s\n", response);
		int send_status = send(fd, response, strlen(response), 0);
		
	}

	close(fd);
	free(arg);
	pthread_exit(NULL);
}

int main() 
{
	// Disable output buffering
	setbuf(stdout, NULL);

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET, 
									 .sin_port = htons(6379),
									 .sin_addr = { htonl(INADDR_ANY) },
								   };

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	while (1)
	{
		printf("Waiting for a client to connect...\n");
		client_addr_len = sizeof(client_addr);
		int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);

		pthread_t pthread;
		pthread_create(&pthread, NULL, (void*) handle_request, &socket_fd);
		pthread_detach(pthread);
		printf("Next request\n");
	}

	printf("Closing connection\n");
	close(server_fd);

	return 0;
}
