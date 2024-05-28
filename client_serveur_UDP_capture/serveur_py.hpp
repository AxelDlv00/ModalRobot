#ifndef SERVEUR_HPP
#define SERVEUR_HPP

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "main.h"

extern float dB[6];
extern char buffer[BUFFER_SIZE];

#define PORT 21000

void * serveur(void * a);

#endif // SERVEUR_HPP
