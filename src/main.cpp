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

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <iterator>

#include "global_parameters.h"
#include "cAlarm.h"
#include "server.h"
#include "log.h"
#include "config.h"
#include "redis_db.h"

#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

volatile bool kinect_alarm_running = true;
struct event_base *base;

int process_request(class cAlarm *alarm, char *command);

void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		kinect_alarm_running = false;
		event_base_loopbreak(base);
	}
}

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {

	class cAlarm *alarm = (class cAlarm*) privdata;
    redisReply *r = (redisReply*) reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY)
    {
    	// Check publication
    	if(r->elements != 3)
    		return;

		if(strncmp(r->element[0]->str,"message",strlen("message")))
			return;
		if(strncmp(r->element[1]->str,"kinectalarm",strlen("kinectalarm")))
			return;

		process_request(alarm, r->element[2]->str);
    }
}

int main(int argc, char** argv)
{

	signal(SIGPIPE, SIG_IGN);
	base = event_base_new();

	redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
	if (c->err) {
		printf("error: %s\n", c->errstr);
		return 1;
	}


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

/*
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
*/

	redisLibeventAttach(c, base);
	redisAsyncCommand(c, onMessage, (void*) &alarma, "SUBSCRIBE kinectalarm");
	event_base_dispatch(base);

	LOG(LOG_NOTICE, "Closing alarm\n");
closing_server:
	deinit_server();
closing_alarm:
	alarma.deinit();
	return retvalue;
}

bool BothAreSpaces(char lhs, char rhs)
{
	return (lhs == rhs) && (lhs == ' ');
}

bool my_predicate(char c)
{
	if(c >= '0' && c <= '9')
		return false;
	if(c >= 'a' && c <= 'z')
		return false;
	if(c >= 'A' && c <= 'Z')
		return false;
	if(c == '-' || c == '+' || c == ' ')
		return false;

	return true;
}



int process_request(class cAlarm *alarm, char *command)
{

	std::string str(command);

	// Remove non wanted characters
	str.erase(std::remove_if(str.begin(), str.end(), my_predicate), str.end());

	// Remove consecutive spaces
	std::string::iterator new_end = std::unique(str.begin(), str.end(), BothAreSpaces);
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
			if(words[1].compare("start") == 0)
				alarm->start_detection();
			else if(words[1].compare("stop") == 0)
				alarm->stop_detection();
			else if(words[1].compare("rst") == 0)
				alarm->reset_detection();
		}
		else if(words[0].compare("lvw") == 0)
		{
			if(words[1].compare("start") == 0)
				alarm->start_liveview();
			else if(words[1].compare("stop") == 0)
				alarm->stop_liveview();
		}
	}
	return 0;
}
