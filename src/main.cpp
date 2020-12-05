/**
 * @author Alejandro Solozabal
 *
 * @file main.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
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

#include "global_parameters.hpp"
#include "alarm.hpp"
#include "log.hpp"
#include "config.hpp"
#include "redis_db.hpp"
#include "common.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define KINECTALARM_VERSION "0.11"

/*******************************************************************
 * Global variables
 *******************************************************************/
volatile bool kinect_alarm_running = true;

/*******************************************************************
 * Function declaration
 *******************************************************************/
int MessageProcess(class Alarm *alarm, char *command);
void OnMessage(redisAsyncContext *c, void *reply, void *privdata);
void* RefreshWatchdog(void *x_void_ptr);

/*******************************************************************
 * Function definition
 *******************************************************************/
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
    /* Handle signals */
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);

    /* Set up syslog */
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog ("kinect_alarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

#ifndef DEBUG
    LOG(LOG_NOTICE, "RELEASE BUILD %s\n", KINECTALARM_VERSION);
#else
    LOG(LOG_NOTICE, "DEBUG BUILD %s\n", KINECTALARM_VERSION);
#endif

    /* Alarm Class creation */
    class Alarm alarm;

    /* Alarm initialization */
    if(alarm.Init())
    {
        LOG(LOG_ERR, "Alarm initialization error\n");
        goto closing_alarm;
    }
    else
    {
        LOG(LOG_NOTICE, "Alarm initialize successful\n");
    }

    /* Init Redis async context */
    if(init_async_redis_db())
        goto closing_alarm;

    /* Subscribe to Redis channel */
    async_redis_subscribe("kinectalarm",OnMessage,&alarm);

    /* Set version on Redis */
    redis_set_char("kinectalarm_version",KINECTALARM_VERSION);

    /* Launch watchdog thread */
    pthread_t watchdog_thread;
    if(pthread_create(&watchdog_thread, NULL, RefreshWatchdog, NULL))
    {
        LOG(LOG_ERR, "Error launching watchdog_thread\n");
        return 1;
    }
    redis_setex_int("kinectalarm_watchdog", 5, 0);

    /* Listen to Redis publications */
    async_redis_event_dispatch();

    LOG(LOG_NOTICE, "Closing alarm\n");

closing_alarm:
    alarm.Term();
    return 0;
}

void OnMessage(redisAsyncContext *c, void *reply, void *privdata)
{
    class Alarm *alarm = (class Alarm*) privdata;
    redisReply *r = (redisReply*) reply;
    if (reply == NULL) return;

    if (r->type == REDIS_REPLY_ARRAY)
    {
        /* Check number of elements */
        if(r->elements != 3)
            return;

        /* Check channel */
        if(strncmp(r->element[0]->str,"message",strlen("message")))
            return;
        if(strncmp(r->element[1]->str,"kinectalarm",strlen("kinectalarm")))
            return;

        /* Process the message */
        MessageProcess(alarm, r->element[2]->str);
    }
}

int MessageProcess(class Alarm *alarm, char *command)
{
    std::string str(command);

    /* Remove non wanted characters */
    str.erase(std::remove_if(str.begin(), str.end(), allowed_characters), str.end());

    /* Remove consecutive spaces */
    std::string::iterator new_end = std::unique(str.begin(), str.end(), both_are_spaces);
    str.erase(new_end, str.end());

    /* Lower cases */
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    /* Remove leading and trailing spaces */
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
                alarm->StartDetection();
            else if(!words[1].compare("stop"))
                alarm->StopDetection();
            else if(!words[1].compare("rst"))
                alarm->ResetDetection();
            else if(!words[1].compare("del")){
                if(words.size() >=3)
                {
                    int value = std::stoi(words[2]);;
                    alarm->DeleteDetection(value);
                }

            }
        }
        else if(!words[0].compare("lvw"))
        {
            if(!words[1].compare("start"))
                alarm->StartLiveview();
            else if(!words[1].compare("stop"))
                alarm->StopLiveview();
        }
        else if(!words[0].compare("tilt"))
        {
            int tilt;
            try {
                tilt = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                tilt = 0;
            }
            alarm->ChangeTilt(tilt);
        }
        else if(!words[0].compare("brightness"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            alarm->ChangeBrightness(value);
        }
        else if(!words[0].compare("contrast"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            alarm->ChangeContrast(value);
        }
        else if(!words[0].compare("threshold"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            alarm->ChangeThreshold(value);
        }
        else if(!words[0].compare("sensitivity"))
        {
            int value;
            try {
                value = std::stoi(words[1]);
            }
            catch (const std::invalid_argument& ia) {
                value = 0;
            }
            alarm->ChangeSensitivity(value);
        }
    }
    return 0;
}

void* RefreshWatchdog(void *x_void_ptr)
{
    while(1)
    {
        redis_setex_int("kinectalarm_watchdog", 2, 0);
        sleep(1);
    }
    return NULL;
}
