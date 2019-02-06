/*
 * log.h
 *
 *  Created on: 5 ene. 2019
 *      Author: asolo
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#ifdef DEBUG_ALARM
	#define LOG(log_level,format, ...) printf(format, ## __VA_ARGS__)
#else
	#define LOG(log_level,format, ...) syslog(log_level,format, ## __VA_ARGS__)
#endif



#endif /* SRC_LOG_H_ */
