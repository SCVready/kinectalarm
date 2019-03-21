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

#include "global_parameters.h"
#include "cAlarm.h"
#include "server.h"
#include "log.h"
#include "config.h"


volatile bool kinect_alarm_running = true;

int process_request(class cAlarm *alarma, char *buff_in,int buff_in_len, char *buff_out, int buff_out_size);

void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		kinect_alarm_running = false;
	}
}

void make_deamon()
{
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
			exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
			exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open any logs here */

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
			/* Log any failures here */
			exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
			/* Log any failures here */
			exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return;
}

int main(int argc, char** argv)
{
	int retvalue = 0;


#ifndef DEBUG_ALARM
	printf("RELEASE BUILD\n");
#else
	printf("DEBUG BUILD\n");
#endif

	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	// Set up syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

#ifndef DEBUG_ALARM
	//make_deamon();
#endif

	// Alarma Class creation
	class cAlarm alarma;

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
	sleep(1);
	return retvalue;
}

int process_request(class cAlarm *alarma, char *buff_in,int buff_in_len, char *buff_out, int buff_out_size)
{
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
			if(!strncmp(buff_in+8,"start",4))
			{
				alarma->start_liveview();
				strncpy(buff_out,"Liveview started",buff_out_size);
			}
			else if(!strncmp(buff_in+8,"stop",4))
			{
				alarma->stop_liveview();
				strncpy(buff_out,"Liveview stopped",buff_out_size);
			}
			else
			{
				strncpy(buff_out,"Liveview command not recognized",buff_out_size);
			}
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
