#ifndef LIST_H
#define LIST_H

// List that stores the number of living processes. Each cell stores the PIDs of processes created together and the number of processes created in the line.
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

list*   initList    ();

cel*    createCel   (unsigned int nProcess, pid_t *processIDs);

void    insertList  (list *l, cel* c);

cel*    findInList  (list *l, pid_t ID);

void    cleanList   (list *l);

#endif