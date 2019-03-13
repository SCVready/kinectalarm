/**
  * @file common.h
  * @author Alejandro Solozabal
  */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

bool check_dir_exist(char *dir);
int create_dir(char *dir);

struct timespec timeAdd(struct timespec t1, struct timespec t2);
struct timespec timeSub(struct timespec t1, struct timespec t2);

#endif /* COMMON_H_ */
