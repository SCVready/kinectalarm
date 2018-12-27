/*
 * server.cpp
 *
 *  Created on: 15 dic. 2018
 *      Author: asolo
 *      Modified from: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include "server.h"

#define PORT 8888
#define MAX_CONNECTIONS 10
#define MAX_PENDING_CONNECTIONS 5
#define SOCKET_PATH "kinect_alarm_socket"
#define BUFFER_SIZE 1024

// Global variables
int server_socket = 0, new_socket = 0, client_socket[MAX_CONNECTIONS] = {0};
struct sockaddr_un address;

int init_server()
{
	int socket_option = true;

	// Create a server socket Unix
	if( (server_socket = socket(AF_UNIX , SOCK_SEQPACKET , 0)) == 0)
	{
		return -1;
	}

	// Set socket option reuseaddr, no needed
	if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&socket_option, sizeof(socket_option)) < 0 )
	{
		return -1;
	}

	// Complete Unix socket info
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path)-1);

	// Remove previous Unix Socket Inode
	unlink((char *)SOCKET_PATH);

	// Bind
	if (bind(server_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		return -1;
	}

	// Listen
	if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int server_loop(class cAlarma *alarma,int (*callback_function)(class cAlarma *,char *, int, char *, int))
{
	int activity, i , valread , sd, addrlen;
	char buffer_in[BUFFER_SIZE],buffer_out[BUFFER_SIZE];
	fd_set readfds;
	int max_sd;
	sigset_t signal_mask;
	struct timespec timeout;
	timeout.tv_sec = 0;
	timeout.tv_nsec = 500000000;

	addrlen = sizeof(address);

	// Mask to disable signal handling of the pselect
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGINT);
	sigaddset(&signal_mask, SIGTERM);
	sigaddset(&signal_mask, SIGQUIT);
	sigprocmask(SIG_BLOCK, &signal_mask, NULL);

	//clear the socket set
	FD_ZERO(&readfds);

	//add master socket to set
	FD_SET(server_socket, &readfds);
	max_sd = server_socket;

	//add child sockets to set
	for ( i = 0 ; i < MAX_CONNECTIONS ; i++)
	{
		//socket descriptor
		sd = client_socket[i];

		//if valid socket descriptor then add to read list
		if(sd > 0)
			FD_SET( sd , &readfds);

		//highest file descriptor number, need it for the select function
		if(sd > max_sd)
			max_sd = sd;
	}

	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	activity = pselect( max_sd + 1 , &readfds , NULL , NULL , &timeout,&signal_mask);

	if ((activity < 0) && (errno!=EINTR))
	{
		printf("select error");
	}

	//If something happened on the master socket ,
	//then its an incoming connection
	if (FD_ISSET(server_socket, &readfds))
	{
		if ((new_socket = accept(server_socket,
				(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		//inform user of socket number - used in send and receive commands
		printf("New connection , socket fd is %d\n" , new_socket);


		//add new socket to array of sockets
		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			//if position is empty
			if( client_socket[i] == 0 )
			{
				client_socket[i] = new_socket;
				printf("Adding to list of sockets as %d\n" , i);

				break;
			}
		}

		if (i == MAX_CONNECTIONS)
		{
			printf("Server full, closing incoming connection\n");
			close(new_socket);
		}
	}

	//else its some IO operation on some other socket
	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		sd = client_socket[i];

		if (FD_ISSET( sd , &readfds))
		{
			//Check if it was for closing , and also read the
			//incoming message
			if ((valread = recv( sd , buffer_in, BUFFER_SIZE,0)) == 0)
			{
				// Client Disconnected
				printf("Host disconnected\n");
				close(sd);
				client_socket[i] = 0;
			}
			else
			{
				// TODO: Function to digest message, execute and response to comand
				buffer_in[valread] = '\0';
				callback_function(alarma,buffer_in,valread,buffer_out,BUFFER_SIZE);
				send(sd , buffer_out , strlen(buffer_out) , 0 );
			}
		}
	}

	return 0;
}

int deinit_server()
{
	for(int i = 0; i < MAX_CONNECTIONS; i++)
	{
		close(client_socket[i]);
	}
	close(server_socket);

	return 0;
}