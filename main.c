#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int separaComando(char *input, char *commands[]);

int main(void) {
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
    printf("Quant Tokens:%d", quantCommands);

   
    return 0;
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