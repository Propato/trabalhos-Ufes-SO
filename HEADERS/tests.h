#ifndef TESTS_H
#define TESTS_H

// ###############  Error Testing Functions  ###############
int     testProcess     (pid_t pid, char *message);

void    testPointers    (void* test, char *message);

void    testInts        (int test, char *message);

#endif