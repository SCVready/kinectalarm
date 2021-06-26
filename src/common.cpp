/**
 * @author Alejandro Solozabal
 *
 * @file common.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "common.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define BILLION 1000000000

/*******************************************************************
 * Function definition
 *******************************************************************/
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

int delete_all_files_from_dir(const char *path)
{
    char command[50];
    sprintf(command, "rm -rf %s/*",path);
    system(command);
    return 0;
}

bool both_are_spaces(char lhs, char rhs)
{
    return (lhs == rhs) && (lhs == ' ');
}

bool allowed_characters(char c)
{
    if(c >= '0' && c <= '9')
        return false;
    if(c >= 'a' && c <= 'z')
        return false;
    if(c >= 'A' && c <= 'Z')
        return false;
    if(c == '-' || c == '+' || c == ' ')
        return false;

    return true;
}

#endif
