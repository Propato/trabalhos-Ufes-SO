#ifndef RUNS_H
#define RUNS_H

// ###########  Functions to Execute Processes  ###########

// Executes the execvp function for processes
void    runProcess      (char *process, int MAX_N_PARAMS);

 // Fork and run the process in foreground
pid_t   runForeground   (char *process, int MAX_N_PARAMS);

// Forks and creates a background group for the processes created together
int     runBackground   (int nProcess, char **process, pid_t *IDs, int MAX_N_PARAMS);

#endif