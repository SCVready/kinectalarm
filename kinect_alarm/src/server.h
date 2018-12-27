/*
 * server.h
 *
 *  Created on: 15 dic. 2018
 *      Author: asolo
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

int init_server();
int server_loop(class cAlarma *alarma,int (*callback_function)(class cAlarma *,char *, int, char *, int));
int deinit_server();


#endif /* SRC_SERVER_H_ */
