/*
 * freenect_test_1.cpp
 *
 *  Created on: 10 ago. 2018
 *      Author: asolo
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include "cAlarma.h"
#include "server.h"

#define FIFO_PATH "/home/pi/kinect_alarm_ctl"

volatile bool kinect_alarm_running = true;

int process_request(class cAlarma *alarma, char *buff_in,int buff_in_len, char *buff_out, int buff_out_size);

void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		kinect_alarm_running = false;
	}
}

int main(int argc, char** argv)
{
	int retvalue = 0;
	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	//Set up syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	class cAlarma alarma;

	if(alarma.init())
	{
		syslog(LOG_ERR, "Alarm initialization error");
		retvalue = -1;
		goto closing_alarm;
	}
	else
		syslog(LOG_NOTICE, "Alarm initialize successful");

	// Initialize server on Unix socket
	if(init_server())
	{
		syslog(LOG_ERR, "Server initialization error");
		retvalue = -1;
		goto closing_server;
	}
	else
			syslog(LOG_NOTICE, "Server initialize successful");

	// Loop readding commands
	while(kinect_alarm_running)
	{
		if(server_loop(&alarma,&process_request))
		{
			syslog(LOG_ERR, "Server error");
			break;
		}
	}

closing_server:
	deinit_server();
closing_alarm:
	alarma.deinit();
	syslog(LOG_NOTICE, "Deinitialize successful");

	return 0;
}

int process_request(class cAlarma *alarma, char *buff_in,int buff_in_len, char *buff_out, int buff_out_size)//BUFF int lengh conten//BUFFout total size
{
	// Prototype
	// digest_request()//perform_request()//response_request()

	if(buff_in_len >= 3 && !strncmp(buff_in,"com",3))
	{
		if(!strncmp(buff_in+4,"det",3))
		{
			if(!strncmp(buff_in+8,"start",4))
			{
				alarma->start_detection();
				strncpy(buff_out,"Detection started",buff_out_size);
			}
			else if(!strncmp(buff_in+8,"stop",4))
			{
				alarma->stop_detection();
				strncpy(buff_out,"Detection stopped",buff_out_size);
			}
			else
			{
				strncpy(buff_out,"Detection command not recognized",buff_out_size);
			}
		}
		else if(!strncmp(buff_in+4,"lvw",3))
		{
			strncpy(buff_out,"Command not implemented",buff_out_size);
		}
		else
		{
			strncpy(buff_out,"Command not recognized",buff_out_size);
		}
	}
	else if (buff_in_len >= 3 && !strncmp(buff_in,"req",3))
	{
		if(!strncmp(buff_in+4,"det",3))
		{
			if(!strncmp(buff_in+8,"status",6))
			{
				if(alarma->is_detection_running())
					strncpy(buff_out,"yes",buff_out_size);
				else
					strncpy(buff_out,"no",buff_out_size);
			}
			else
			{
				strncpy(buff_out,"Detection Request not recognized",buff_out_size);
			}
		}
		else if(!strncmp(buff_in+4,"lvw",3))
		{
			if(!strncmp(buff_in+8,"status",6))
			{
				if(alarma->is_liveview_running())
					strncpy(buff_out,"yes",buff_out_size);
				else
					strncpy(buff_out,"no",buff_out_size);
			}
			else
			{
				strncpy(buff_out,"Liveview Request not recognized",buff_out_size);
			}
		}
		else
		{
			strncpy(buff_out,"Request not recognized",buff_out_size);
		}
	}
	else
	{
		strncpy(buff_out,"Action not recognized",buff_out_size);
	}

	return 0;
}

