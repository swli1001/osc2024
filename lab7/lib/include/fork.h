#ifndef FORK_H
#define FORK_H

void forkProcedureForChild();
int copyCurrentTask();
int syscall_fork();

void fork_test();

#endif