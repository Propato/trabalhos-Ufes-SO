#define MAX_N_PARAMS 3 // process's name + max params 


// ###########  Functions to Execute Processes  ###########
void    runProcess      (char *process);
pid_t   runForeground   (char *process);
int     runBackground   (int nProcess, char **process, pid_t *IDs);
