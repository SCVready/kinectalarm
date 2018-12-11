/**
  * @file common.cpp
  * @author Alejandro Solozabal
  */

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

int create_dir(char* path)
{
	if(mkdir(path, 0770) == -1)
	{
		printf("Failed to create %s\n", path);
		perror("mkdir");
		return -1;
	}
	else
		printf("Created directory: %s\n", path);
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
