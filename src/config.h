/*
 * config.h
 *
 *  Created on: 22 ene. 2019
 *      Author: asolo
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <string>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "global_parameters.h"
#include "cAlarm.h"

#define MY_ENCODING "UTF-8"


//Structs



int write_conf_file(struct sDet_conf det_conf,struct sLvw_conf lvw_conf,const char *path);
int parse_conf_file(struct sDet_conf *det_conf,struct sLvw_conf *lvw_conf,const char *path);

#endif /* SRC_CONFIG_H_ */
