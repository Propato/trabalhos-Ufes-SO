#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../HEADERS/tests.h"

/*
    #########################################################
    ###############  Error Testing Functions  ###############
    #########################################################
*/

int testProcess(pid_t pid, char *message){ 
    
    if(kill(pid, 0) == 0){
        return 1;
    } else
    if (errno == ESRCH){
        errno = 0;
    } else 
        testInts(-1, message);
    return 0;
}

void testPointers(void* test, char *message){ 
    if(test == NULL){
        printf("%s\n", message);
        exit(2);
    }
}
void testInts(int test, char *message){ 
    if(test < 0){
        if(errno == ECHILD)
            return;
        
        perror(message);
        exit(1);
    }
}
