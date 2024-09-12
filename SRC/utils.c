#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../HEADERS/utils.h"

int splitString(char *buffer, char **process, char *delimiter, int MAX){

    int n = 0;
    process[n++] = strtok(buffer, delimiter);
    if(!process[0])
        return 0;

    while(n < MAX && (process[n] = strtok(NULL, delimiter)))
        { n++; }

    return n;
}