#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "HEADERS/list.h"       // List to store information about processes created by the shell
#include "HEADERS/runs.h"       // Functions to execute processes, in foreground and background
#include "HEADERS/tests.h"      // Test functions to check for errors
#include "HEADERS/utils.h"      // Function to handle input strings

// ####################  Process Rules  ####################
#define MAX_N_PROCESS 5         // 1 foreground + 4 background process
#define MAX_N_PARAMS 3          // process's name + max params 

// #################  Internal Functions  #################
void    die             ();
void    waitall         ();

// #############  Childrens's Signals Handler  #############
void    sendSignal      (pid_t pid, int signal);
void    checkSigChld    (pid_t pid, int status);
void    handleSigChld   (int sig);

// #############  Shell's Signals Handler  #############
void    handlerSigInt   (int sig);
void    handlerSigTstp  (int sig);

// #################  Secondary Functions  #################
void    setActions      ();
void    cleanAll        ();

/*
    #########################  Main  #########################
*/

list *listProcess = NULL;
char **process = NULL;
char *buffer = NULL;

int main(int argc, char **argv){

    /* *****************  Init  Variables  ***************** */
    size_t size = 30;
    int nProcess = 0;
    pid_t IDs[MAX_N_PROCESS];

    process = malloc(sizeof(char*) * MAX_N_PROCESS);
    testPointers(process, "Error Malloc -> process");
    
    buffer = malloc(sizeof(char) * size);
    testPointers(buffer, "Error Malloc -> buffer");

    /* *****************  Initialize List  ***************** */
    listProcess = initList();

    /* ****************  Set  Sig Handlers  **************** */
    setActions();

    while(1){
        printf("fsh> ");

        /* *************  Get & Handle  Input  ************* */
        size_t new_size = getline(&buffer, &size, stdin);
        if(new_size > size) size = new_size;
        if(new_size == 1) continue;
        /* RASCUNHO: SERVE PARA FINALIZAR FACILMENTE A EXECUÇÃO DO PROGRAMA */
        if(new_size == 2) break;
        testInts(new_size-1, "Error getLine");
        buffer[new_size-1] = '\0';


        if(!strcmp(buffer, "die")){
            die();
            break;
        }
        if(!strcmp(buffer, "waitall")){
            waitall();
            continue;
        }

        nProcess = splitString(buffer, process, "#", MAX_N_PROCESS);

        /* *****************  Run Process  ***************** */
        // Both if's are a protection for correct memory and process management, in case a sent process does not work properly.
        IDs[0] = runForeground(process[0], MAX_N_PARAMS); // Return the son's pid
        if (IDs[0] == 0){
            cleanAll();
            return 3;
        }
        if (runBackground(nProcess-1, &process[1], &IDs[1], MAX_N_PARAMS)){
            cleanAll();
            return 4;
        }

        /* RASCUNHO: AUXILIAR NOS TESTES, IMPRIMINDO OS IDS */
        printf("IDs:");
        for(int i=0; i< nProcess; i++)
            printf(" %d", IDs[i]);
        printf("\n");
        
        /* *************  Stores Process  IDs  ************* */
        insertList(listProcess, createCel(nProcess, IDs));

        int status=0;
        testInts(waitpid(IDs[0], &status, 0), "Error WaitPID");
        checkSigChld(IDs[0], status);
    }
    cleanAll();
  return 0;
}

/*
    #########################################################
    #################   Internal Functions  #################
    #########################################################
*/

/*
    Internal function that terminates the shell and kills all processes before
*/
void die() {
    printf("fsh> Killing Shell and Children...\n");
    if(listProcess)
        for(cel *c=listProcess->first; c != NULL && listProcess->nProcessAlive; c = c->next){
            sendSignal(c->processIDs[0], SIGTERM);
        }
}

/*
    Internal function that releases all zombie descendants
*/
void waitall() { 
    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        checkSigChld(pid, status);
    }

    if(errno == EINTR){
        printf("fsh> There are still zombie child processes.\n");
        errno = 0;
    }
    else if(errno)
        testInts(-1, "Error Wait All");        
}


/*
    #########################################################
    #############  Childrens's Signals Handler  #############
    #########################################################
*/

/*
    Function to propagate the signal from a process to processes created on the same line.
*/
void sendSignal(pid_t pid, int signal){
    cel* c = findInList(listProcess, pid);
    if(!c)
        return;

    if(testProcess(c->processIDs[0], "Error Test Kill Foreground")){
        testInts(kill(c->processIDs[0], signal), "Error Sinal Foreground");
        
        if(!testProcess(c->processIDs[0], "Error Count Kill Foreground"))
            listProcess->nProcessAlive--; // If the process was alive and is not anymore, the count drops.
    }

    int alive = 0;    
    if(c->nProcess > 1 && testProcess(-c->processIDs[1], "Error Kill Background")){
            
        // Count how many processes are still and continue to be alive
        for(int i = 1; i < c->nProcess; i++)
            if(testProcess(c->processIDs[i], "Error Test Kill Background"))
                alive++;

        testInts(kill(-c->processIDs[1], signal), "Error Sinal Background");

        for(int i = 1; i < c->nProcess; i++)
            if(testProcess(c->processIDs[i], "Error Count Kill Background"))
                alive--;

        listProcess->nProcessAlive -= alive;
    }
}

/*
    Function to check if the process was killed or suspended by a signal and send the signal to the other process created together.
*/
void checkSigChld(pid_t pid, int status){
    if (WIFSIGNALED(status)) {
        listProcess->nProcessAlive--;
        sendSignal(pid, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        sendSignal(pid, WSTOPSIG(status));
    } else if (WIFEXITED(status)) {
        listProcess->nProcessAlive--;
    }
}

/*
    Check the children that have finished.
*/
void handleSigChld(int sig){
    int status;
    pid_t pid;

    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        checkSigChld(pid, status);
    }
}

/*
    #########################################################
    ##############   Shell's Signals Handlers  ##############
    #########################################################
*/

/*
    For some reason when 'n' is typed there are some bugs.
*/
void handlerSigInt(int sig){ //Tratador do SIGINT
    if (listProcess && !listProcess->nProcessAlive){

        printf("\nfsh> There are live children processes. Terminate the shell? (y/n):\n");
        
        size_t size = 2;
        size = getline(&buffer, &size, stdin);
        if(buffer[0] == 'y' || buffer[0] == 'Y'){
            cleanAll();
            exit(0); // Kill shell but not the children.
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    } else {
        printf("\nfsh> Ending...\n");
        cleanAll();
        exit(0);
    }
}

/*
    When Ctrl+Z is typed, stops the children.
*/
void handlerSigTstp(int sig){

    if(!listProcess || !listProcess->nProcessAlive){
        printf("\nfsh> SIGTSTP (Ctrl+Z): No child processes.\nfsh>");
        return;
    }

    for(cel *c=listProcess->first; c != NULL && listProcess->nProcessAlive; c = c->next){
        sendSignal(c->processIDs[0], sig);
    }
    printf("\nfsh> SIGTSTP (Ctrl+Z): Stopped children processes.\nfsh>");
}

/*
    #########################################################
    #################  Secondary Functions  #################
    #########################################################
*/

/*
    Defines Sigaction to handle signals.
*/
void setActions(){

    struct sigaction actionSigChild;
    actionSigChild.sa_handler = &handleSigChld;
    actionSigChild.sa_flags = SA_RESTART;
    sigemptyset(&actionSigChild.sa_mask);

    testInts(sigaction(SIGCHLD, &actionSigChild, NULL), "Error Sig Child Action");

    struct sigaction actionSigInt;
    actionSigInt.sa_handler = &handlerSigInt;
    actionSigInt.sa_flags = SA_RESTART;
    sigemptyset(&actionSigInt.sa_mask);

    testInts(sigaction(SIGINT, &actionSigInt, NULL), "Error Sig Int Action");

    struct sigaction actionSigTstp;
    actionSigTstp.sa_handler = &handlerSigTstp;
    sigemptyset(&actionSigTstp.sa_mask);
    actionSigTstp.sa_flags = SA_RESTART;

    testInts(sigaction(SIGTSTP, &actionSigTstp, NULL), "Error Sig TStp Action");
}

/*
    Functions to free program memory
*/
void cleanAll(){
    if(process)
    free(process);
    process = NULL;

    if(buffer)
    free(buffer);
    buffer = NULL;

    cleanList(listProcess);
    listProcess = NULL;
}
