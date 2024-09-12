#ifndef RUNS_H
#define RUNS_H


// ###########  Functions to Execute Processes  ###########
void    runProcess      (char *process, int MAX_N_PARAMS);

pid_t   runForeground   (char *process, int MAX_N_PARAMS);

int     runBackground   (int nProcess, char **process, pid_t *IDs, int MAX_N_PARAMS);

#endif