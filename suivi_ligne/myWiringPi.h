#ifndef WIRING_H
#define WIRING_H
#include <stdio.h> //fct de base
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h> //pour thread
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include <poll.h>


int mydigitalRead(int n);

void mydigitalWrite(int n,int val);

int exists(const char *fname);

void exportPin(int n);

void unexportPin(int n);

void pinModeOut(int n);

void pinModeIn(int n);

int mywaitForInterrupt (int pin, int mS);

#endif
