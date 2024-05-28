#include "recup_consigne.h"
#include <thread>

//float dB[6];
//char buffer[BUFFER_SIZE];

void * recup_consigne(void * a) {
        char message[BUFFER_SIZE];
        
        std::cout<<"Recuperation en cours"<<std::endl;
        
        float dB_temp[6];
	
        while (true) {
                strcpy(message, buffer);
                int taille = strlen(message);
                if (taille > 0){                        
                        // Separe les valeurs
                        
                        int result = sscanf(message, "Vecteur : %f, %f, %f, %f, %f, %f;",&dB_temp[0], &dB_temp[1], &dB_temp[2], &dB_temp[3], &dB_temp[4], &dB_temp[5] );
                        if (result == 6){
                                for (int i=0; i<6; i++){
					dB[i] = dB_temp[i];
				}
                                //std::cout<<"Reçu ! " << std::endl;
                        }
                        else {
                                
                                std::cout<<"Erreur de format : "<< message << std::endl;
                        }
                } 
                //printf("dB : %f, %f, %f, %f, %f; %f\n", &dB[0], dB[1], dB[2], dB[3], dB[4], dB[5]);
		calcul_angle(dB);
		avancer(dB);
        
        }
}


