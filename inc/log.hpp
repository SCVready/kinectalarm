/**
 * @author Alejandro Solozabal
 *
 * @file log.h
 *
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdio.h>
#include <syslog.h>

/*******************************************************************
 * Macros
 *******************************************************************/
#ifdef NDEBUG
    #define LOG(log_level,format, ...) syslog(log_level,format, ## __VA_ARGS__)
#else
    #define LOG(log_level,format, ...) printf(format, ## __VA_ARGS__)
#endif



#endif /* SRC_LOG_H_ */
