/*
 * redis_db.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: scvready
 */


#include "redis_db.h"

//Global variables
redisContext *c				= NULL;
redisAsyncContext *c_async	= NULL;
pthread_mutex_t redis_context_mutex;
struct event_base *base;


// Sync functions
int init_redis_db()
{
	// Connect to the redis DB
	c = redisConnectUnix("/tmp/redis.sock");
	if (c == NULL || c->err)
	{
	    if (c)
	    	LOG(LOG_ERR,"init_redis_db(): %s\n", c->errstr);
	    else
	    	LOG(LOG_ERR,"init_redis_db(): Can't allocate redis context\n");

	    return -1;
	}
	// Initialize context mutex
	if (pthread_mutex_init(&redis_context_mutex, NULL) != 0)
		return -1;

	return 0;
}

int deinit_redis_db()
{
	redisFree(c);
	return 0;
}

// GET functions
int redis_get_int(char *key, int *value)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"GET %s",key);
    if(reply->type != REDIS_REPLY_STRING)
    {
    	retval = -1;
    	goto clean;
    }

	*value = atoi(reply->str);

clean:
	freeReplyObject(reply);
	pthread_mutex_unlock(&redis_context_mutex);
	return 0;
}

int redis_get_char(char *key, char **value)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"GET %s",key);
    if(reply->type != REDIS_REPLY_STRING)
    {
    	retval = -1;
    	goto clean;
    }

    *value = (char*) malloc((reply->len + 1) * sizeof(char));
	strncpy(*value,reply->str,reply->len);

clean:
	freeReplyObject(reply);
	pthread_mutex_unlock(&redis_context_mutex);
	return 0;
}

//TODO float,byte_array

// SET functions

int redis_set_int(char *key, int value)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"SET %s %d",key,value);
    if(reply->type == REDIS_REPLY_ERROR)
    {
    	retval = -1;
    	goto clean;
    }

clean:
    freeReplyObject(reply);
    pthread_mutex_unlock(&redis_context_mutex);
    return retval;
}

int redis_set_char(char *key, char *value)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"SET %s %s",key,value);
    if(reply->type == REDIS_REPLY_ERROR)
    {
    	retval = -1;
    	goto clean;
    }

clean:
    freeReplyObject(reply);
    pthread_mutex_unlock(&redis_context_mutex);
    return 0;
}

// SETEX functions
int redis_setex_int(char *key, int time, int value)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"SETEX %s %d %d",key,time,value);
    if(reply->type == REDIS_REPLY_ERROR)
    {
    	retval = -1;
    	goto clean;
    }

clean:
    freeReplyObject(reply);
    pthread_mutex_unlock(&redis_context_mutex);
    return retval;
}


// Publish

int redis_publish(char *channel, char *message)
{
	int retval = 0;
	redisReply *reply;

	pthread_mutex_lock(&redis_context_mutex);

    reply = (redisReply *) redisCommand(c,"PUBLISH %s %s",channel,message);
    if(reply->type == REDIS_REPLY_ERROR)
    {
    	retval = -1;
    	goto clean;
    }

clean:
    freeReplyObject(reply);
    pthread_mutex_unlock(&redis_context_mutex);
	return 0;
}

// Async functions
int init_async_redis_db()
{
	signal(SIGPIPE, SIG_IGN);
	base = event_base_new();

	c_async = redisAsyncConnectUnix(REDIS_UNIXSOC);
	if (c_async->err) {
		LOG(LOG_ERR,"init_async_redis_db(): %s\n", c->errstr);
		return 1;
	}
	redisLibeventAttach(c_async, base);
	return 0;
}

int async_redis_subscribe(char * channel, void callback(redisAsyncContext *c, void *reply, void *privdata),void * data)
{
	redisAsyncCommand(c_async, callback, data, "SUBSCRIBE %s",channel);
	return 0;
}

int async_redis_event_dispatch()
{
	event_base_dispatch(base);
	return 0;
}
int async_redis_event_loopbreak()
{
	event_base_loopbreak(base);
	return 0;
}



