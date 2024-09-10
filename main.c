#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "HEADERS/list.h"

#define MAX_N_PROCESS 5
#define MAX_N_PARAMS 3 // process's name + max params 

void runProcess(char *process);
pid_t runForeground(char *process);
int runBackground(int nProcess, char **process, pid_t *IDs);

void sendSignal(pid_t pid, int signal);
void handle_sigchld(int sig);

// void waitBackground();
int splitString(char *buffer, char **process, char *delimiter, int MAX);

void testPointers(void* test, char *message);
void testInts(int test, char *message);

// Função auxiliar para printar informações dos processos
void printAll(int i){
    pid_t pid = getpid();    // Obtém o ID do processo atual
    pid_t pgid = getpgid(0); // Obtém o PGID do processo atual
    pid_t sid = getsid(0);   // Obtém o SID do processo atual

    printf("Process: %d\n", i);
    printf("PID: %d\n", pid);
    printf("PGID: %d\n", pgid);
    printf("SID: %d\n", sid);
}

list *listProcess = NULL;

int main(int argc, char **argv){

    size_t size = 30;
    int nProcess = 0;

    // printf("Shell Process:\n");
    // printAll();

    char **process = malloc(sizeof(char*) * MAX_N_PROCESS);
    testPointers(process, "Error Malloc -> process");
    
    char *buffer = malloc(sizeof(char) * size);
    testPointers(buffer, "Error Malloc -> buffer");

    pid_t IDs[MAX_N_PROCESS];

    struct sigaction sigChild;
    sigChild.sa_handler = &handle_sigchld;
    sigChild.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sigChild.sa_mask);

    sigaction(SIGINT, &sigChild, NULL);
    testInts(sigaction(SIGCHLD, &sigChild, NULL), "Error SigAction");

    while(1){
        printf("fsh> ");
        size_t new_size = getline(&buffer, &size, stdin);
        if(new_size == 1)
            continue;
        /* APENAS TESTE, SERVE PARA FINALIZAR FACILMENTE A EXECUÇÃO DO PROGRAMA */
        if(new_size == 2) 
            break;
        testInts(new_size-1, "Error getLine");
        buffer[new_size-1] = '\0';

        nProcess = splitString(buffer, process, "#", MAX_N_PROCESS);

        // Both if's are a protection for correct memory and process management, in case a sent process does not work properly.
        pid_t foreID = runForeground(process[0]);
        if (foreID == 0){
            free(process);
            free(buffer);
            cleanList(listProcess);
            return 3;
        }
        IDs[0] = foreID;
        if (runBackground(nProcess-1, &process[1], &IDs[1])){
            free(process);
            free(buffer);
            cleanList(listProcess);
            return 4;
        }

        printf("IDs:");
        for(int i=0; i< nProcess; i++)
            printf(" %d", IDs[i]);
        printf("\n");
        
        listProcess = insertList(listProcess, nProcess, IDs);
        int status=0;
        testInts(waitpid(foreID, &status, 0), "Error WaitPID");
        if (WIFSIGNALED(status))
            sendSignal(foreID, WTERMSIG(status));
        else if (WIFEXITED(status)){
            findInList(listProcess, foreID);
        }
    }

    free(process);
    free(buffer);
    cleanList(listProcess);
  return 0;
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
        testInts((pid=fork()), "Error Fork Background Process");
        if(pid > 0){
            IDs[i] = pid;
            if(i==0)
                gpID = pid;
            continue;
        }
        testInts(setpgid(0, gpID), "Error setPGID Background");

        /* Sons */
        testInts((pid=fork()), "Error Fork Nested Background Process");
        // printAll(i);
        runProcess(process[i]);
        return 4;
    }
    return 0;
}

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

void handle_sigchld(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFSIGNALED(status)) {
            sendSignal(pid, WTERMSIG(status));
        } else if (WIFEXITED(status)){
            findInList(listProcess, pid);
        }
    }
}

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