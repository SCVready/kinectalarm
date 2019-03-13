/*
 * server.h
 *
 *  Created on: 15 dic. 2018
 *      Author: asolo
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

// Includes

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <syslog.h>
#include "log.h"

int init_server();
int server_loop(class cAlarm *alarma,int (*callback_function)(class cAlarm *,char *, int, char *, int));
int deinit_server();

void lvw_frame_send();

#endif /* SRC_SERVER_H_ */
