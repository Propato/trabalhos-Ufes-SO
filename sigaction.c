#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_N_PROCESS 5
#define MAX_N_PARAMS 3 // process's name + max params 

void runProcess(char *process);
pid_t runForeground(char *process, pid_t pgid);
int runBackground(int n_process, char **process, pid_t pgid);
void waitProcesses(pid_t pgid);  // Função para esperar processos com base no grupo de processos
int splitString(char *buffer, char **process, char *delimiter, int MAX);

void testPointers(void* test, char *message);
void testInts(int test, char *message);

void sigint_handler_shell(int sig);  // Tratador para SIGINT na shell
void sigtstp_handler_shell(int sig); // Tratador para SIGTSTP na shell

pid_t foreground_pgid = -1; // Armazena o PGID do grupo de processos em foreground
int active_processes = 0;   // Contador de processos ativos (foreground e background)

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

    // Instalando tratadores de sinais para SIGINT e SIGTSTP na shell
    struct sigaction sa_int, sa_tstp;
    sa_int.sa_handler = sigint_handler_shell;
    sa_tstp.sa_handler = sigtstp_handler_shell;
    sigemptyset(&sa_int.sa_mask);
    sigemptyset(&sa_tstp.sa_mask);
    sa_int.sa_flags = 0;
    sa_tstp.sa_flags = 0;

    sigaction(SIGINT, &sa_int, NULL);    // Captura SIGINT apenas na shell
    sigaction(SIGTSTP, &sa_tstp, NULL);  // Captura SIGTSTP para suspender os processos filhos

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

        // Criar um grupo de processos com base no primeiro processo (foreground)
        pid_t pgid = 0;  // O primeiro processo vai definir o PGID do grupo

        // Executar o processo em foreground
        pid_t foreID = runForeground(process[0], pgid);
        if (foreID == 0){
            free(process);
            free(buffer);
            return 3;
        }
        // Definir o PGID se for o primeiro processo
        if (pgid == 0) pgid = foreID;

        // Incrementar o número de processos ativos
        active_processes++;

        // Executar os processos em background
        int feedback = runBackground(n_process-1, &process[1], pgid);
        if (feedback >= 0){
            free(process);
            free(buffer);
            return feedback;
        }

        // Aumentar o contador de processos ativos de background
        active_processes += (n_process - 1);

        // Esperar pelos processos do grupo inteiro (foreground e background)
        waitProcesses(pgid);  
    }

    free(process);
    free(buffer);
    return 0;
}

pid_t runForeground(char *process, pid_t pgid){
    pid_t pid;
    testInts((pid=fork()), "Error Fork Foreground");

	if(pid == 0){
        // Ignorar SIGINT nos processos filhos
        struct sigaction sa_ignore;
        sa_ignore.sa_handler = SIG_IGN;  // Ignorar SIGINT
        sigaction(SIGINT, &sa_ignore, NULL);

        if (pgid == 0) {
            pgid = getpid();  // O primeiro processo define o PGID
        }
        setpgid(0, pgid);  // Define o grupo de processos do foreground

        printAll(-1);
        runProcess(process);
    } else {
        // Definir o PGID do grupo de foreground
        if (pgid == 0) {
            pgid = pid;  // Definir o PGID do primeiro processo
        }
        setpgid(pid, pgid);  // Coloca o processo no grupo
        foreground_pgid = pgid;
    }
    return pid;
}

int runBackground(int n_process, char **process, pid_t pgid){
    if(n_process == 0)
        return -1;

    pid_t pid;

    for (int i = 0; i < n_process; i++){
        //Criando novo Processo
        testInts((pid=fork()), "Error Fork Background Process");

        /* Sons */
        if(pid == 0){
            testInts((pid=fork()), "Error Fork Nested Background Process");

            // Ignorar SIGINT nos processos de background
            struct sigaction sa_ignore;
            sa_ignore.sa_handler = SIG_IGN;  // Ignorar SIGINT
            sigaction(SIGINT, &sa_ignore, NULL);

            setpgid(0, pgid);  // Coloca todos os processos no mesmo grupo

            printAll(i);
            runProcess(process[i]);
            return 4;
        } else {
            setpgid(pid, pgid);  // Coloca o processo no grupo de background
        }
    }
    return 0;
}

void runProcess(char *process){
    int argc = 0;
    char **argv = malloc(sizeof(char*) * (MAX_N_PARAMS+1)); // +1 for the NULL at the end.
    testPointers(argv, "Error Malloc -> argv");

    argc = splitString(process, argv, " \n\t\v\f\r", MAX_N_PARAMS);

    argv[argc] = NULL;

    execvp(argv[0], argv);
    printf("execvp ERRO - process invalid: %s\n", argv[0]);
    free(argv);
}

void waitProcesses(pid_t pgid){
    int status;
    pid_t pid;

    // Esperar por todos os processos no grupo (foreground e background)
    while ((pid = waitpid(-pgid, &status, 0)) > 0) {
        if (WIFEXITED(status)) {
            printf("Processo %d finalizado normalmente com status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
            printf("Processo %d finalizado por sinal %d\n", pid, signal);

            // Propagar o sinal para todos os processos no grupo
            kill(-pgid, signal);  // Enviar o mesmo sinal para o grupo inteiro
        }

        // Reduzir o número de processos ativos
        active_processes--;
    }

    if (pid == -1 && errno == ECHILD) {
        printf("Todos os processos finalizados.\n");
    } else {
        perror("waitpid error");
    }
}

int splitString(char *buffer, char **process, char *delimiter, int MAX){
    int n = 0;
    process[n++] = strtok(buffer, delimiter);
    while(n < MAX && (process[n] = strtok(NULL, delimiter))){
        n++;
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

// Tratador de SIGINT (Ctrl+C) apenas para a shell
void sigint_handler_shell(int sig) {
    if (active_processes > 0) {
        // Se houver processos vivos, perguntar ao usuário
        printf("\nShell: Há processos vivos. Deseja encerrar a shell? (y/n): ");
        char resposta = getchar();
        if (resposta == 'y' || resposta == 'Y') {
            exit(0); // Encerrar a shell
        }
    } else {
        // Se não houver processos vivos, encerrar diretamente
        printf("\nShell: Encerrando.\n");
        exit(0);
    }
}

// Tratador de SIGTSTP (Ctrl+Z) apenas para a shell, suspende processos filhos
void sigtstp_handler_shell(int sig) {
    if (foreground_pgid != -1) {
        // Enviar o sinal SIGTSTP para todo o grupo de processos (foreground e background)
        kill(-foreground_pgid, SIGTSTP);
        printf("\nShell: Processos filhos suspensos com SIGTSTP.\n");
    } else {
        printf("\nShell: Recebido SIGTSTP (Ctrl+Z), mas sem processos no foreground.\n");
    }
}
