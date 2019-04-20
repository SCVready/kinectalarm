/*
 * redis_db.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: scvready
 */


#include "redis_db.h"

//Global variables
redisContext *c = NULL;
pthread_mutex_t redis_context_mutex;

int init_redis_db()
{
	// Connect to the redis DB
	c = redisConnect(REDIS_IP, REDIS_PORT);
	if (c == NULL || c->err)
	{
	    if (c)
	        printf("Error: %s\n", c->errstr);
	    else
	        printf("Can't allocate redis context\n");

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
