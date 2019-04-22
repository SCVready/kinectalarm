/*
 * redis_db.h
 *
 *  Created on: Apr 17, 2019
 *      Author: scvready
 */

#ifndef REDIS_DB_H_
#define REDIS_DB_H_

#include <hiredis/hiredis.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define REDIS_IP			"127.0.0.1"
#define REDIS_PORT			6379
#define REDIS_MAX_STRING	200


int init_redis_db();
int deinit_redis_db();

// GET functions
int redis_get_int(char *key, int *value);
int redis_get_char(char *key, char **value);

// SET functions
int redis_set_int(char *key, int value);
int redis_set_char(char *key, char *value);

// PUBLISH
int redis_publish(char *channel, char *message);

#endif /* REDIS_DB_H_ */
