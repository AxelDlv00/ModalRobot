#include "serveur_py.hpp"

//char buffer[BUFFER_SIZE];

void * serveur(void * a)
{
    std::cout << "Lancement du serveur UDP..." << std::endl;

    socklen_t fromlen;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        std::cerr << "Erreur de création du socket" << std::endl;
    }

    // Lui-meme
    struct sockaddr_in server;
    bzero(&server, sizeof(server));

    // Configure l'adresse du serveur
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        std::cerr << "Erreur de binding" << std::endl;
    }
    fromlen = sizeof(struct sockaddr_in);

    std::cout << "Serveur UDP en écoute sur le port 21000 !" << std::endl;

    int n;
    struct sockaddr_in from;
    while (true) 
    {
        // Reception
        n = recvfrom(sock, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&from, &fromlen);
        buffer[n] = '\0';

        if (n < 0) 
        {
            std::cerr << "Erreur de reception" << std::endl;
        }
        else if (n > 0)
        {
            //std::cout << "Reçu ! " << std::endl;
            //std::cout << "Client : " << buffer << std::endl;
        }
        bzero(buffer,BUFFER_SIZE);
    }

    return 0;
}


