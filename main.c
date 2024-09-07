#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>


//MAx quant de argunmentos do comando +2
const int MaxCmdLength = 250;   


void executaProcessoBackground(char **commands, int amountCommands);
void executaProcessoForeground(bool isForeground, char *command);
void runCommand(char *command);

int separaComando(char *input, char *commands[]);
void testError(int test, char *funcString);






int main(void){
    printf("fsh>");
    char *commands[5];
    char linha[150];
     
    //Lendo os comandos
    if(fgets(linha, sizeof(linha), stdin) == NULL){
        printf("Erro Leitura dos Comandos !!");
        exit(1);
    }
    linha[strcspn(linha, "\n")] = '\0';
    int quantCommands = separaComando(linha, commands);

    for(int i=0; i<quantCommands; i++){
        printf("Token %d: %s\n", i, commands[i]);
    }

    pid_t pid = fork();

    //Verificando Erro
    if(pid < 0){
        printf("Erro ao Criar Fork");
        exit(1);
    }
    if(pid == 0){
        executaProcessoForeground(true, commands[0]);
        executaProcessoBackground(commands, quantCommands);

    }




/*
    ExecutaProcesso(true, commands[0]);
    for(int i=1; i<quantCommands; i++){
        executaProcessoForeground(false, commands[i]);
    } 

    char *args[] = {"ps", NULL};
    execvp(args[0], args);

    //printf("Quant Tokens:%d", quantCommands);

*/
    return 0;
}



void executaProcessoBackground(char **commands, int amountCommands){
    //Criando nova Sessão
    if(setsid() == -1){
        printf("setsId Error");
        exit(1);
    }
/*
    FILE* null_file = fopen("/dev/null", "w");
    if (null_file == NULL) {
        perror("Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }

    // Redirect stdout and stderr to /dev/null
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
*/
    //Obtendo pid do Pai
    pid_t pgid = getpid();

    for (int i = 1; i < amountCommands; i++){
        //Criando novo Processo
        pid_t pid = fork();
        testError(pid, "fork  Error");

        if(pid == 0){
            
            pid_t pid2 = fork();
            testError(pid2, "fork  Error");
            if(pid2 == 0){
                setpgid(0, pgid);
                runCommand(commands[i]);
            }

            setpgid(0, pgid);
            runCommand(commands[i]);
        }
    }
    
    return;
}


void executaProcessoForeground(bool isForeground, char *command){    
    pid_t pid = fork();

    //Verificando Erro
    if(pid < 0){
        printf("Erro ao Criar Fork");
        exit(1);
    
    //Processo Filho
    }else if(pid == 0){
        runCommand(command);
    
    }else{
        printf("Processo pai com PID %d\n", getpid()); 
        if(isForeground){
            wait(NULL); // Aguarda o término do processo filho
        }
    }
    
    return ;
}


void runCommand(char *command){
    //printf("Processo filho com PID %d\n", getpid());
    
    //Argumentos do comando
    char *argv[MaxCmdLength];
    int argc =0;

    //Separando argumentos do Comando
    char *token1 = strtok(command, " \t\n");
    while (token1 != NULL){
        argv[argc++] = token1;
        token1 = strtok(NULL, " \n\t");
    }
    
    //setando NULL na ultima posição
    argv[argc] = NULL;
    
    //Executando o Comando
    if(execvp(argv[0],argv) != -1){
        printf("Função execvp ERRO, command:%s\n", argv[0]);
        exit(1);
    }
    return;
}







int separaComando(char *input, char *commands[]){
    char *token;
    int quantTokens = 0;
    
    token = strtok(input, "#");
    
    for(quantTokens = 0; token != NULL && quantTokens<5; quantTokens++){
        commands[quantTokens] = token;
        token = strtok(NULL, "#");
    }

    return quantTokens;
}


void testError(int test, char *funcString){
    if(test == -1){
        printf("%s", funcString);
        exit(1);
    }
    return;
}