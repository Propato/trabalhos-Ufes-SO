#include <stdlib.h>
#include <unistd.h>

#include "../HEADERS/list.h"

/*
typedef struct {
    pid_t *processIDs;
    unsigned int nProcess;
    unsigned int *states;

    list *next;
} LIST *list;
*/

list* insertList(list *l, unsigned int nProcess, pid_t *processIDs){
    
    list *cel = malloc(sizeof(list));
    cel->nProcess = nProcess;

    cel->processIDs = malloc(sizeof(pid_t)*nProcess);
    pid_t *aux = cel->processIDs;
    
    cel->states = malloc(sizeof(char)*nProcess);
    char *states = cel->states;

    for(int i=0; i<nProcess; i++){
        *aux = *processIDs;
        *states = 1;

        aux++;
        states++;
        processIDs++;
    }

    cel->next = l;

    return cel;
}

list* findInList(list *l, pid_t ID){

    for(list *cel=l; cel != NULL; cel=cel->next){

        if(!cel->processIDs || !cel->states)
            continue;

        pid_t *aux = cel->processIDs;
        char *states = cel->states;

        for(int i=0; i<cel->nProcess; i++){
            if(*aux == ID && *states==1){
                *states = 0;
                return cel;
            }
            aux++;
            states++;
        }
    }
    return NULL;
}

void setStates(list *l){
    if(!l || !l->states)
        return;

    char *states = l->states;

    for(int i=0; i<l->nProcess; i++){
        *states = 0;
        states++;
    }
}

void cleanList(list *l){
    if(!l)
        return;

    if(l->processIDs)
        free(l->processIDs);
    if(l->states)
        free(l->states);
    for(list *cel=l->next; cel != NULL; cel=cel->next){
        if(cel->processIDs)
            free(cel->processIDs);
        if(cel->states)
            free(cel->states);
        free(l);
        l = cel;
    } free(l);
}