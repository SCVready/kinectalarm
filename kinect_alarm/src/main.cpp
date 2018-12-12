/*
 * freenect_test_1.cpp
 *
 *  Created on: 10 ago. 2018
 *      Author: asolo
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include "cAlarma.h"

void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		/*
		alarma.running = false;
		pthread_cond_signal(&cKinect::depth_ready);
		pthread_cond_signal(&cKinect::video_ready);
		*/
	}
}

int main(int argc, char** argv)
{
	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	//Set syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	class cAlarma alarma;

	if(alarma.init())
		syslog(LOG_ERR, "Initialization error");
	else
		syslog(LOG_NOTICE, "Initialize successful");

	sleep(2);
	if(!alarma.start_detection())
		syslog(LOG_INFO, "Detection started");
	else
		syslog(LOG_INFO, "Detection already started");

	sleep(2);
	if(!alarma.stop_detection())
		syslog(LOG_INFO, "Detection stopped");
	else
		syslog(LOG_INFO, "Detection already stopped");

	sleep(2);
	if(!alarma.start_detection())
		syslog(LOG_INFO, "Detection started");
	else
		syslog(LOG_INFO, "Detection already started");

	sleep(2);
	if(!alarma.stop_detection())
		syslog(LOG_INFO, "Detection stopped");
	else
		syslog(LOG_INFO, "Detection already stopped");

	alarma.deinit();
	syslog(LOG_NOTICE, "Deinitialize successful");

	return 0;
}
