#include <stdlib.h>
#include <unistd.h>

#include "../HEADERS/list.h"

/*
struct CEL {
    pid_t *processIDs;
    unsigned int nProcess;

    struct CEL *next;
};

typedef struct CEL cel;

struct LIST {
    unsigned int nProcessAlive;

    struct CEL *first;
    struct CEL *last;
};

typedef struct LIST list;
*/

list* initList(){
    list *l = malloc(sizeof(list));

    l->nProcessAlive = 0;

    l->first = NULL;
    l->last = NULL;

    return l;
}

cel* createCel(unsigned int nProcess, pid_t *processIDs){
    if(!processIDs || !nProcess) return NULL;

    cel *c = malloc(sizeof(cel));
    c->nProcess = nProcess;

    c->processIDs = malloc(sizeof(pid_t) * nProcess);
    pid_t *auxIDs = c->processIDs;
    
    for(int i=0; i<nProcess; i++){
        *auxIDs = *processIDs;

        auxIDs++;
        processIDs++;
    }

    c->next = NULL;

    return c;
}

void insertList(list *l, cel *c){
    if(!l || !c) return;
    
    if(!l->first){
        l->first = c;
        l->last = c;
        l->nProcessAlive = c->nProcess;
        return;
    }

    l->last->next = c;
    l->last = c;
    l->nProcessAlive += c->nProcess;
}

cel* findInList(list *l, pid_t ID){
    if(!l || !l->first || !ID) return NULL;


    for(cel *c=l->first; c != NULL; c=c->next){

        if(!c->processIDs)
            continue;

        pid_t *auxIDs = c->processIDs;

        for(int i=0; i<c->nProcess; i++){
            if(*auxIDs == ID )
                return c;

            auxIDs++;
        }
    }
    return NULL;
}

void cleanCel(cel *c){
    if(!c) return;

    if(c->processIDs)
        free(c->processIDs);
    free(c);
}

void cleanList(list *l){
    if(!l) return;

    cel *aux, *c;
    for(c = l->first; c != NULL; c = aux){
        aux = c->next;
        cleanCel(c);
    }
    free(l);
}