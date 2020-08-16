/**
 * @author Alejandro Solozabal
 *
 * @file redis_db.hpp
 *
 */

#ifndef REDIS_DB_H_
#define REDIS_DB_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include "log.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define REDIS_UNIXSOC		"/tmp/redis.sock"
#define REDIS_MAX_STRING	200

/*******************************************************************
 * Function declaration
 *******************************************************************/
int init_redis_db();
int deinit_redis_db();

int redis_get_int(char *key, int *value);
int redis_get_char(char *key, char **value);

int redis_set_int(char *key, int value);
int redis_set_char(char *key, char *value);

int redis_setex_int(char *key, int time, int value);

int redis_publish(char *channel, char *message);

int init_async_redis_db();
int async_redis_subscribe(char * channel, void callback(redisAsyncContext *c, void *reply, void *privdata),void * data);
int async_redis_event_dispatch();
int async_redis_event_loopbreak();

#endif /* REDIS_DB_H_ */
