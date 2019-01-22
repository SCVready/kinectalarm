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
#include "log.h"
#include "config.h"

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

	//TODO
	//// LibXML2 test
	struct sDet_conf det_conf={false,2000,10,5,0};
	write_conf_file(det_conf,"config.xml");
	parse_conf_file(&det_conf,"config.xml");
	////

	int retvalue = 0;

	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	// Set up syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	// Alarma Class creation
	class cAlarma alarma;

	// Alarm initialization
	if(alarma.init())
	{
		LOG(LOG_ERR, "Alarm initialization error\n");
		retvalue = -1;
		goto closing_alarm;
	}
	else
		LOG(LOG_NOTICE, "Alarm initialize successful\n");

	// Initialize server on Unix socket
	if(init_server())
	{
		LOG(LOG_ERR, "Server initialization error\n");
		retvalue = -1;
		goto closing_server;
	}
	else
		LOG(LOG_NOTICE, "Server initialize successful\n");

	// Loop reading commands
	while(kinect_alarm_running)
	{
		if(server_loop(&alarma,&process_request))
		{
			LOG(LOG_ERR, "Server error\n");
			break;
		}
	}

closing_server:
	deinit_server();
closing_alarm:
	alarma.deinit();
	LOG(LOG_NOTICE, "Deinitialize successful\n");
	return retvalue;
}

int process_request(class cAlarma *alarma, char *buff_in,int buff_in_len, char *buff_out, int buff_out_size)//BUFF int lengh conten//BUFFout total size
{
	// Prototype TODO
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
			else if(!strncmp(buff_in+8,"rst",3))
			{
				alarma->reset_detection();
				strncpy(buff_out,"Number of detection to 0",buff_out_size);
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
			else if(!strncmp(buff_in+8,"num",3))
			{
				snprintf(buff_out,buff_in_len,"%d",alarma->get_num_detections());
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
