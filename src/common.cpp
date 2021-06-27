/**
 * @author Alejandro Solozabal
 *
 * @file common.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <cerrno>

#include "common.hpp"

/*******************************************************************
 * Function definition
 *******************************************************************/
int CreateDirectory(const char* path)
{
    int ret_val = 0;

    if(-1 == mkdir(path, 0770))
    {
        if(errno != EEXIST)
        {
            ret_val = -1;
        }
    }
    return ret_val;
}

int DeleteAllFilesFromDirectory(const char *path)
{
    char command[50];
    sprintf(command, "rm -rf %s/*",path);
    system(command);
    return 0;
}

bool BothAreSpaces(char lhs, char rhs)
{
    return (lhs == rhs) && (lhs == ' ');
}

bool AllowedCharacters(char c)
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
