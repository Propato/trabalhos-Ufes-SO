#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "HEADERS/list.h"

// ####################  Process Rules  ####################
#define MAX_N_PROCESS 5 // 1 foreground + 4 background process
#define MAX_N_PARAMS 3 // process's name + max params 

// ###########  Functions to Execute Processes  ###########
void    runProcess      (char *process);
pid_t   runForeground   (char *process);
int     runBackground   (int nProcess, char **process, pid_t *IDs);

// ###################  Signal Handlers  ###################
void    sendSignal      (pid_t pid, int signal);
void    checkSigChld    (pid_t pid, int status);
void    handleSigChld   (int sig);

// #################  Secondary Functions  #################
int     splitString     (char *buffer, char **process, char *delimiter, int MAX);
void    setActions       ();
void    cleanAll        (char **process, char *buffer, list *listProcess)

// ###############  Error Testing Functions  ###############
void    testPointers    (void* test, char *message);
void    testInts        (int test, char *message);

/*
    #########################  Main  #########################
*/

list *listProcess = NULL;

int main(int argc, char **argv){

    /************ Init Variables ************/
    size_t size = 30;
    int nProcess = 0;
    pid_t IDs[MAX_N_PROCESS];

    char **process = malloc(sizeof(char*) * MAX_N_PROCESS);
    testPointers(process, "Error Malloc -> process");
    
    char *buffer = malloc(sizeof(char) * size);
    testPointers(buffer, "Error Malloc -> buffer");

    /************ Set Sig Handlers ************/
    setActions();

    while(1){
        printf("fsh> ");

        /************ Get & Handle Input ************/
        size_t new_size = getline(&buffer, &size, stdin);
        if(new_size == 1)
            continue;
        /* RASCUNHO: SERVE PARA FINALIZAR FACILMENTE A EXECUÇÃO DO PROGRAMA */
        if(new_size == 2) 
            break;
        testInts(new_size-1, "Error getLine");
        buffer[new_size-1] = '\0';

        nProcess = splitString(buffer, process, "#", MAX_N_PROCESS);

        /************ Run Process ************/
        // Both if's are a protection for correct memory and process management, in case a sent process does not work properly.
        IDs[0] = runForeground(process[0]); // Return the son's pid
        if (IDs[0] == 0){
            cleanAll(process, buffer, listProcess);
            return 3;
        }
        if (runBackground(nProcess-1, &process[1], &IDs[1])){
            cleanAll(process, buffer, listProcess);
            return 4;
        }

        /* RASCUNHO: AUXILIAR NOS TESTES, IMPRIMINDO OS IDS */
        printf("IDs:");
        for(int i=0; i< nProcess; i++)
            printf(" %d", IDs[i]);
        printf("\n");
        
        /************ Stores Process IDs ************/
        listProcess = insertList(listProcess, nProcess, IDs);

        int status=0;
        testInts(waitpid(IDs[0], &status, 0), "Error WaitPID");
        checkSigChld(IDs[0], status);
    }

    cleanAll(process, buffer, listProcess);
  return 0;
}

/*
    #########################################################
    ###########   Functions to Execute Processes  ###########
    #########################################################
*/

void runProcess(char *process){
    //printf("Processo filho com PID %d\n", getpid());

    // printf("Process: %s\n", process);
    
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

pid_t runForeground(char *process){
    pid_t pid;
    testInts((pid=fork()), "Error Fork Foreground");

	if(pid == 0){
        // printAll(-1);
        runProcess(process);
    }
    return pid;
}

int runBackground(int nProcess, char **process, pid_t *IDs){
    if(nProcess == 0)
        return 0;

    // Necessario um 'pai' para os processos em background para que eles possam estar no mesmo grupo e sessao, pois o pai só morre depois de criar todos os filhos dentro da mesma sessao e grupo.
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
        runProcess(process[i]);
        return 4;
    }
    return 0;
}

/*
    #########################################################
    ###################  Signal Handlers  ###################
    #########################################################
*/

void sendSignal(pid_t pid, int signal){
    list* process = findInList(listProcess, pid);
    if(!process)
        return;

    if(process->states[0])
        testInts(kill(process->processIDs[0], signal), "Error Sinal Foreground");
    
    setStates(process);
    if(process->nProcess > 1){
        if(kill(-process->processIDs[1], 0) == 0){
            testInts(kill(-process->processIDs[1], signal), "Error Sinal Background");
            printf("PID: %d Foreground: %d Group: %d\n", pid, process->processIDs[0], process->processIDs[1]);
        } else if (errno == ESRCH) // O grupo de processos não existe
            errno = 0;
    }
    // sleep(20);
}

void checkSigChld(pid_t pid, int status){
    if (WIFSIGNALED(status)) {
        sendSignal(pid, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        sendSignal(pid, WSTOPSIG(status));
    } else if (WIFEXITED(status)){
        findInList(listProcess, pid); // Find in List also sets the process state as 0.
    }
}

void handleSigChld(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        checkSigChld(pid, status);
    }
}

/*
    #########################################################
    #################  Secondary Functions  #################
    #########################################################
*/

int splitString(char *buffer, char **process, char *delimiter, int MAX){

    int n = 0;
    process[n++] = strtok(buffer, delimiter);
    if(!process[0])
        return 0;

    while(n < MAX && (process[n] = strtok(NULL, delimiter))){
        n++;
        // printf("process[%d] = %s.\n", n, process);
    }
    return n;
}

void setActions(){

    struct sigaction sigChild;
    sigChild.sa_handler = &handleSigChld;
    sigChild.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sigChild.sa_mask);

    testInts(sigaction(SIGCHLD, &sigChild, NULL), "Error SigAction");
}

void cleanAll(char **process, char *buffer, list *listProcess){
    free(process);
    process = NULL;

    free(buffer);
    buffer = NULL;

    cleanList(listProcess);
    listProcess = NULL;
}

/*
    #########################################################
    ###############  Error Testing Functions  ###############
    #########################################################
*/

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