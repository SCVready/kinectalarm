/*
 * freenect_test_1.cpp
 *
 *  Created on: 10 ago. 2018
 *      Author: asolo
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "cKinect.h"
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

	class cAlarma alarma;

	alarma.init();

	alarma.run();
	sleep(10);
	alarma.stop();
	sleep(10);
	alarma.run();
	sleep(10);
	alarma.stop();

	alarma.deinit();


	return 0;
}
