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

#define FIFO_PATH "/home/pi/kinect_alarm_ctl"

volatile bool kinect_alarm_running = true;

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
	// Variables
	int fifo_fd =-1;
	char read_fifo[80];

	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	//Set up syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	class cAlarma alarma;

	if(alarma.init())
		syslog(LOG_ERR, "Initialization error");
	else
		syslog(LOG_NOTICE, "Initialize successful");

	// Fifo creation
	mkfifo(FIFO_PATH, 0666);
	fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);

	// Loop reading comands from fifo
	while(kinect_alarm_running)
	{
		usleep(200000);
		if (read(fifo_fd, read_fifo, sizeof(read_fifo)) <= 0)
			continue;
		else
		{
			if(strncmp(read_fifo,"0",1))
			{
				if(!alarma.start_detection())
					syslog(LOG_INFO, "Detection started");
				else
					syslog(LOG_INFO, "Detection already started");
			}
			else if(strncmp(read_fifo,"1",1))
			{
				if(!alarma.stop_detection())
					syslog(LOG_INFO, "Detection stopped");
				else
					syslog(LOG_INFO, "Detection already stopped");
			}
			else
				continue;
		}
	}
	close(fifo_fd);

	alarma.deinit();
	syslog(LOG_NOTICE, "Deinitialize successful");

	return 0;
}
