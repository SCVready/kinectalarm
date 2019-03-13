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
