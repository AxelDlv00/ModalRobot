#ifndef SERVEUR_HPP
#define SERVEUR_HPP

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 26000
#define BUFFER_SIZE 1024

void * serveur(void * a);
void * recup_consigne(void * a);

// Déclaration du buffer global
extern char buffer[BUFFER_SIZE];


#endif // SERVEUR_HPP
