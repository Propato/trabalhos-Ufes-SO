#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "HEADERS/list.h" // Lista para armazenar os PID dos processos, quantidade de processos por linha e quantidade de processos vivos
#include "HEADERS/runs.h" // Funções para executar processos em foreground e background
#include "HEADERS/tests.h" // Funções de teste para verificar erros
#include "HEADERS/utils.h" // Função para separar linha de entrada contendo os comandos

// ####################  Process Rules  ####################
#define MAX_N_PROCESS 5 // 1 foreground + 4 background process

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
        IDs[0] = runForeground(process[0]); // Return the son's pid
        if (IDs[0] == 0){
            cleanAll();
            return 3;
        }
        if (runBackground(nProcess-1, &process[1], &IDs[1])){
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

void die() {
    printf("fsh> Killing Shell and Children...\n");
    if(listProcess)
        for(cel *c=listProcess->first; c != NULL && listProcess->nProcessAlive; c = c->next){
            sendSignal(c->processIDs[0], SIGTERM);
        }
}

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
            
        // Contabilizar quantos processos ainda estão e continuam vivos
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

void checkSigChld(pid_t pid, int status){
    if (WIFSIGNALED(status)) {
        sendSignal(pid, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        sendSignal(pid, WSTOPSIG(status));
    } else if (WIFEXITED(status)) {
        listProcess->nProcessAlive--;
    }
}

void handleSigChld(int sig){
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        checkSigChld(pid, status);
    }
}

/*
    #########################################################
    ##############   Shell's Signals Handlers  ##############
    #########################################################
*/

void handlerSigInt(int sig){
    if (listProcess && !listProcess->nProcessAlive){

        printf("\nfsh> There are live children processes. Terminate the shell? (y/n): ");
        char resposta = getchar();
        if (resposta == 'y' || resposta == 'Y') {
            cleanAll();
            exit(0); // Kill shell but not the children.
        }
    } else {
        printf("\nfsh> Ending...\n");
        cleanAll();
        exit(0);
    }
}

void handlerSigTstp(int sig){

    if(!listProcess || !listProcess->nProcessAlive){
        printf("\nfsh> SIGTSTP (Ctrl+Z): No child processes.\n");
        return;
    }

    for(cel *c=listProcess->first; c != NULL && listProcess->nProcessAlive; c = c->next){
        sendSignal(c->processIDs[0], sig);
    }
    printf("\nfsh> SIGTSTP (Ctrl+Z): Stopped children processes.\n");
}

/*
    #########################################################
    #################  Secondary Functions  #################
    #########################################################
*/

void setActions(){

    struct sigaction actionSigChild;
    actionSigChild.sa_handler = &handleSigChld;
    actionSigChild.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&actionSigChild.sa_mask);

    testInts(sigaction(SIGCHLD, &actionSigChild, NULL), "Error Sig Child Action");

    struct sigaction actionSigInt;
    actionSigInt.sa_handler = &handlerSigInt;
    actionSigInt.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&actionSigInt.sa_mask);

    testInts(sigaction(SIGINT, &actionSigInt, NULL), "Error Sig Int Action");

    struct sigaction actionSigTstp;
    actionSigTstp.sa_handler = &handlerSigTstp;
    sigemptyset(&actionSigTstp.sa_mask);
    actionSigTstp.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    testInts(sigaction(SIGTSTP, &actionSigTstp, NULL), "Error Sig TStp Action");
}

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
