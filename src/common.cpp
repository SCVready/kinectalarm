/**
  * @file common.cpp
  * @author Alejandro Solozabal
  */

#include "common.h"

int create_dir(char* path)
{
	if(mkdir(path, 0770) == -1)
		return -1;
	else
		return 0;
}

bool check_dir_exist(char *path)
{
	DIR* dir;
	if((dir = opendir(path)))
	{
		closedir(dir);
		return true;
	}
	else
		return false;
}

#define BILLION 1000000000
struct timespec timeAdd(struct timespec t1, struct timespec t2)
{
    long sec = t2.tv_sec + t1.tv_sec;
    long nsec = t2.tv_nsec + t1.tv_nsec;
    if (nsec >= BILLION) {
        nsec -= BILLION;
        sec++;
    }
    return (struct timespec){ .tv_sec = sec, .tv_nsec = nsec };
}
struct timespec timeSub(struct timespec t1, struct timespec t2)
{
    long sec = t1.tv_sec - t2.tv_sec;
    long nsec = t1.tv_nsec - t2.tv_nsec;
    long res = sec*BILLION + nsec;
    return (struct timespec){ .tv_sec = 0, .tv_nsec = res };
}
