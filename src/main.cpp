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
#include <pthread.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <iterator>

#include "global_parameters.h"
#include "cAlarm.h"
#include "log.h"
#include "config.h"
#include "redis_db.h"
#include "common.h"

volatile bool kinect_alarm_running = true;

int message_process(class cAlarm *alarm, char *command);
void onMessage(redisAsyncContext *c, void *reply, void *privdata);
void *refresh_watchdog(void *x_void_ptr);

void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		kinect_alarm_running = false;
		async_redis_event_loopbreak();
	}
}

int main(int argc, char** argv)
{

#ifndef DEBUG_ALARM
	printf("RELEASE BUILD\n");
#else
	printf("DEBUG BUILD\n");
#endif

	int retvalue = 0;


	// Handle signals
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	// Set up syslog
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

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

	// INIT REDIS
	if(init_async_redis_db())
		goto closing_alarm;

	// Subscribe to redis channel
	async_redis_subscribe("kinectalarm",onMessage,&alarma);

	//TODO LAUNCH THREAD
	pthread_t watchdog_thread;

	if(pthread_create(&watchdog_thread, NULL, refresh_watchdog, NULL))
	{
		LOG(LOG_ERR, "Error launching watchdog_thread\n");
		return 1;
	}
	redis_setex_int("kinectalarm_watchdog", 5, 0);

	// Listen to publishes
	async_redis_event_dispatch();

	LOG(LOG_NOTICE, "Closing alarm\n");

closing_alarm:
	alarma.deinit();
	return retvalue;
}


void onMessage(redisAsyncContext *c, void *reply, void *privdata) {

	class cAlarm *alarm = (class cAlarm*) privdata;
    redisReply *r = (redisReply*) reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY)
    {
    	// Check number of elements
    	if(r->elements != 3)
    		return;

    	// Check channel
		if(strncmp(r->element[0]->str,"message",strlen("message")))
			return;
		if(strncmp(r->element[1]->str,"kinectalarm",strlen("kinectalarm")))
			return;

		// Process the message
		message_process(alarm, r->element[2]->str);
    }
}

int message_process(class cAlarm *alarm, char *command)
{
	std::string str(command);

	// Remove non wanted characters
	str.erase(std::remove_if(str.begin(), str.end(), allowed_characters), str.end());

	// Remove consecutive spaces
	std::string::iterator new_end = std::unique(str.begin(), str.end(), both_are_spaces);
	str.erase(new_end, str.end());

	// Lower cases
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	// Remove leading and trailing spaces
	if(str.front() == ' ')
		str.erase(0,1);
	if(str.back() == ' ')
		str.erase(str.length()-1,1);

	std::istringstream iss(str);
	std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

	if(words.size() >=2)
	{
		if(words[0].compare("det") == 0)
		{
			if(!words[1].compare("start"))
				alarm->start_detection();
			else if(!words[1].compare("stop"))
				alarm->stop_detection();
			else if(!words[1].compare("rst"))
				alarm->reset_detection();
			else if(!words[1].compare("del")){
				if(words.size() >=3)
				{
					int value = std::stoi(words[2]);;
					alarm->delete_detection(value);
				}

			}
		}
		else if(!words[0].compare("lvw"))
		{
			if(!words[1].compare("start"))
				alarm->start_liveview();
			else if(!words[1].compare("stop"))
				alarm->stop_liveview();
		}
		else if(!words[0].compare("tilt"))
		{
			int tilt = std::stoi(words[1]);
			alarm->change_tilt(tilt);
		}
		else if(!words[0].compare("brightness"))
		{
			int value = std::stoi(words[1]);
			alarm->change_brightness(value);
		}
		else if(!words[0].compare("contrast"))
		{
			int value = std::stoi(words[1]);
			alarm->change_contrast(value);
		}
		else if(!words[0].compare("threshold"))
		{
			int value = std::stoi(words[1]);
			alarm->change_threshold(value);
		}
		else if(!words[0].compare("sensitivity"))
		{
			int value = std::stoi(words[1]);
			alarm->change_sensitivity(value);
		}
	}
	return 0;
}


void *refresh_watchdog(void *x_void_ptr)
{
	while(1){
	redis_setex_int("kinectalarm_watchdog", 2, 0);
	sleep(1);
	}
	return NULL;
}
