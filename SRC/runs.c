#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../HEADERS/runs.h"
#include "../HEADERS/tests.h"
#include "../HEADERS/utils.h"

/*
    #########################################################
    ###########   Functions to Execute Processes  ###########
    #########################################################
*/

void runProcess(char *process, int MAX_N_PARAMS){    
    int argc = 0;
    char **argv = malloc(sizeof(char*) * (MAX_N_PARAMS+1)); // +1 for the NULL at the end.
    testPointers(argv, "Error Malloc -> argv");

    argc = splitString(process, argv, " \n\t\v\f\r", MAX_N_PARAMS);
    // setando NULL na ultima posição
    argv[argc] = NULL;
    
    // Executando o Comando
    if(argc)
        execvp(argv[0], argv);
    printf("execvp ERRO - process invalid: %s\n", argv[0]);
    free(argv);
}

pid_t runForeground(char *process, int MAX_N_PARAMS){
    pid_t pid;
    testInts((pid=fork()), "Error Fork Foreground");

	if(pid == 0){
        // printAll(-1);
        runProcess(process, MAX_N_PARAMS);
    }
    return pid;
}

int runBackground(int nProcess, char **process, pid_t *IDs, int MAX_N_PARAMS){
    if(nProcess == 0)
        return 0;

    pid_t pid, gpID=0;

    for (int i = 0; i < nProcess; i++){
        //Criando novo Processo
        testInts((pid=fork()), "Error Fork Background");
        if(pid > 0){
            IDs[i] = pid;
            if(i==0)
                gpID = pid;
            continue;
        }
        testInts(setpgid(0, gpID), "Error setPGID Background");

        /* Sons */
        testInts((pid=fork()), "Error Fork Nested Background");
        // printAll(i);
        runProcess(process[i], MAX_N_PARAMS);
        return 4;
    }
    return 0;
}