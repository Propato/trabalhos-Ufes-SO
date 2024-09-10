#ifndef LIST_H
#define LIST_H

struct LIST {
    pid_t *processIDs;
    unsigned int nProcess;
    char *states;

    struct LIST *next;
};

typedef struct LIST list;

list* insertList(list *l, unsigned int nProcess, pid_t *processIDs);

list * findInList(list *l, pid_t ID);

void setStates(list *l);

void cleanList(list *l);

#endif