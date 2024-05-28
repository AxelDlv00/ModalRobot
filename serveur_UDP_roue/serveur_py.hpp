#ifndef SERVEUR_HPP
#define SERVEUR_HPP

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 21000

void * serveur(void * a);

// Déclaration du buffer global
extern char buffer[BUFFER_SIZE];


#endif // SERVEUR_HPP
