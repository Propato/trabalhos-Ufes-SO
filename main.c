#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_N_PROCESS 5
#define MAX_N_PARAMS 3 // process's name + max params 

void runProcess(char *process);
pid_t runForeground(char *process);
int runBackground(int n_process, char **process);

void waitBackground();
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

int main(int argc, char **argv){

    size_t size = 30;
    int n_process = 0;

    // printf("Shell Process:\n");
    // printAll();

    char **process = malloc(sizeof(char*) * MAX_N_PROCESS);
    testPointers(process, "Error Malloc -> process");
    
    char *buffer = malloc(sizeof(char) * size);
    testPointers(buffer, "Error Malloc -> buffer");

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

        n_process = splitString(buffer, process, "#", MAX_N_PROCESS);

        // Both if's are a protection for correct memory and process management, in case a sent process does not work properly.
        pid_t foreID = runForeground(process[0]);
        if (foreID == 0){
            free(process);
            free(buffer);
            return 3;
        }
        int feedback = runBackground(n_process-1, &process[1]);
        if (feedback >= 0){
            free(process);
            free(buffer);
            return feedback;
        }

        testInts(waitpid(foreID, NULL, 0), "Error WaitPID");
    }

    free(process);
    free(buffer);
  return 0;
}

pid_t runForeground(char *process){
    pid_t pid;
    testInts((pid=fork()), "Error Fork Foreground");

	if(pid == 0){
        printAll(-1);
        runProcess(process);
    }
    return pid;
}

int runBackground(int n_process, char **process){
    if(n_process == 0)
        return -1;

    // Necessario um 'pai' para os processos em background para que eles possam estar no mesmo grupo e sessao, pois o pai só morre depois de criar todos os filhos dentro da mesma sessao e grupo.
    pid_t pid;
    testInts((pid=fork()), "Error Fork Background Father");
    if(pid != 0)
        return -1;

    testInts(setsid(), "Error setSID");

    for (int i = 0; i < n_process; i++){
        //Criando novo Processo
        testInts((pid=fork()), "Error Fork Background Process");

        /* Sons */
        if(pid == 0){
            testInts((pid=fork()), "Error Fork Nested Background Process");
            printAll(i);
            runProcess(process[i]);
            return 4;
        }
    }
    waitBackground(); // função para esperar todos os processos em background morrerem, checando seus status e sinais recebidos (talvez seja mais interessante que o sigaction)
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
    execvp(argv[0], argv);
        printf("execvp ERRO - process invalid: %s\n", argv[0]);
        free(argv);
}

void waitBackground(){
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, 0)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child %d killed by signal %d\n", pid, WTERMSIG(status));
        }
    }

    if (pid == -1 && errno == ECHILD) {
        printf("No more children to wait for.\n");
    } else {
        perror("waitpid error");
    }
}

int splitString(char *buffer, char **process, char *delimiter, int MAX){

    int n = 0;
    process[n++] = strtok(buffer, delimiter);
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
        perror(message);
        exit(1);
    }
}