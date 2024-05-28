#include "client.hpp"

char buffer_envoi[BUFFER_SIZE];

void * client(void * a) {
    int sockfd;
    struct sockaddr_in serverAddr;

    // Crée un socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Erreur de création du socket" << std::endl;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    // Configure l'adresse du serveur
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_CLIENT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.0.104");//"192.168.0.104"); // Remplacez par l'adresse IP du serveur si nécessaire

    while (true) {
        
        std::string message = "Angle : " + std::to_string(angle_consigne) + "," + std::to_string(test_avancer) + ";\n";
        strncpy(buffer_envoi, message.c_str(), BUFFER_SIZE);
        buffer_envoi[BUFFER_SIZE - 1] = '\0'; // S'assurer que le buffer est null-terminated

        // Envoie le message au serveur
        sendto(sockfd, (const char *)buffer_envoi, strlen(buffer_envoi), MSG_CONFIRM, (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
        std::cout << buffer_envoi << std::endl;

        bzero(buffer_envoi,BUFFER_SIZE);
        usleep(100);
    }

    close(sockfd);
}

	
