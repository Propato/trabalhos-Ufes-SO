#ifndef TESTS_H
#define TESTS_H

// ###############  Error Testing Functions  ###############

// Test whether the pid matches a live process or group
int     testProcess     (pid_t pid, char *message);

// Test for malloc errors
void    testPointers    (void* test, char *message);

// Test errors of several functions with integer return: > 0 == sucess
void    testInts        (int test, char *message);

#endif