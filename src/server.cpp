/*
 * server.cpp
 *
 *  Created on: 15 dic. 2018
 *      Author: asolo
 *      Modified from: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */


#include "server.h"

#define PORT 8888
#define MAX_CONNECTIONS 10
#define MAX_PENDING_CONNECTIONS 5

#define BUFFER_SIZE 1024

// Global variables

static int server_socket = 0, new_socket = 0, client_socket[MAX_CONNECTIONS] = {0};
static struct sockaddr_un address;
static sigset_t signal_mask;
static struct timespec timeout;
static char buffer_in[BUFFER_SIZE];
static char buffer_out[BUFFER_SIZE];
static int addrlen;

int init_server()
{
	// Syslog initialization
	openlog ("kinect_alarm::server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	int socket_option = true;

	// Create a server socket Unix
	if( (server_socket = socket(AF_UNIX , SOCK_SEQPACKET , 0)) == 0)
	{
		LOG(LOG_ERR,"Error opening the socket\n");
		return -1;
	}

	// Set socket option reuseaddr, no needed
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&socket_option, sizeof(socket_option)) < 0 )
	{
		LOG(LOG_ERR,"Error on setsockopt\n");
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
		LOG(LOG_ERR,"Error on bind\n");
		return -1;
	}

	// Listen
	if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0)
	{
		LOG(LOG_ERR,"Error on listen\n");
		return -1;
	}

	// Mask to disable signal handling of the pselect
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGINT);
	sigaddset(&signal_mask, SIGTERM);
	sigaddset(&signal_mask, SIGQUIT);
	sigprocmask(SIG_BLOCK, &signal_mask, NULL);

	// pselect timeout
	timeout.tv_sec = 0;
	timeout.tv_nsec = 500000000;

	addrlen = sizeof(address);

	return 0;
}

int server_loop(class cAlarm *alarma,int (*callback_function)(class cAlarm *,char *, int, char *, int))
{
	int activity, i , valread , sd;
	fd_set readfds;
	int max_fd;
	int out_param;

	// Clear the fd_set
	FD_ZERO(&readfds);

	// Add master socket to fd_set
	FD_SET(server_socket, &readfds);
	max_fd = server_socket;

	// Add child sockets to fd_set
	for ( i = 0 ; i < MAX_CONNECTIONS ; i++)
	{
		sd = client_socket[i];

		// If valid socket descriptor then add to read list
		if(sd > 0)
			FD_SET( sd , &readfds);

		// Get the highest fd, need it for the pselect function
		if(sd > max_fd)
			max_fd = sd;
	}

	//wait for an activity on one of the sockets , timeout is 0.5s ,
	activity = pselect( max_fd + 1 , &readfds , NULL , NULL , &timeout,&signal_mask);

	if ((activity < 0) && (errno!=EINTR))
	{
		LOG(LOG_ERR,"Error on pselect\n");
		return -1;
	}

	if(activity == 0)
		return 0;

	//If something happened on the master socket ,
	//then its an incoming connection
	if (FD_ISSET(server_socket, &readfds))
	{
		if ((new_socket = accept(server_socket,
				(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
		{
			LOG(LOG_ERR,"Error on accept\n");
			return -1;
		}

		//inform user of socket number - used in send and receive commands
		LOG(LOG_DEBUG,"New connection\n");

		//add new socket to array of sockets
		for (i = 0; i < MAX_CONNECTIONS; i++)
		{
			//if position is empty
			if( client_socket[i] == 0 )
			{
				client_socket[i] = new_socket;
				break;
			}
		}

		if (i == MAX_CONNECTIONS)
		{
			LOG(LOG_NOTICE,"Server full, closing incoming connection\n");
			close(new_socket);
		}
	}

	//else its some IO operation on some other socket
	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		sd = client_socket[i];
		out_param = 0;
		if (sd > 0 && FD_ISSET( sd , &readfds))
		{
			// Read socket
			if ((valread = recv( sd , buffer_in, BUFFER_SIZE,0)) == 0)
			{
				// Client Disconnected
				LOG(LOG_DEBUG,"Host disconnected\n");
				close(sd);
				client_socket[i] = 0;
			}
			else
			{
				// Read message
				buffer_in[valread] = '\0';
				callback_function(alarma,buffer_in,valread,buffer_out,BUFFER_SIZE);
				send(sd , buffer_out , strlen(buffer_out) , 0 );

				// Treat out_parameters
				//TODO
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
