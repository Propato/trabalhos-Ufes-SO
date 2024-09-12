#ifndef UTILS_H
#define UTILS_H

// Function to separate a string into others.
// Used to separate processes ('#') and to separate the process from its parameters (' ').
int     splitString     (char *buffer, char **process, char *delimiter, int MAX);

#endif