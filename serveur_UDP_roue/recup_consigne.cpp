#include "serveur_udp.hpp"
#include "recup_consigne.h"
#include <thread>

#include <cmath>  // pour fmod et M_PI

extern float V_Consigne_Globale;
extern float angle_consigne;
float angle_reference = 3.14/3;

float V1, V2, V3;

float convertirAngle(float angle) {
    // Vérification si l'angle est supérieur à pi
    if (angle >= M_PI) {
        angle -= 2 * M_PI;
	return convertirAngle(angle);
    }
    else if (angle < - M_PI) {
        angle += 2 * M_PI;
	return convertirAngle(angle);
    }
    return angle;
}


void * recup_consigne(void * a) {
        char message[BUFFER_SIZE];
        
        float V1_temp, V2_temp, V3_temp;
        float angle_consigne_temp;
        V1 = 0.5f;
        while (true) {
                strcpy(message, buffer);
		//std::cout<<"Hey"<<std::endl;
		//std::cout<<message<<std::endl;
                
                int taille = strlen(message);
                if (taille > 0){                        
                        // Separe les valeurs
			int test = 0;
                        int result = sscanf(message, "Angle : %f, %i",&angle_consigne_temp, &test );
                        
                        /*int result = sscanf(message, "Vecteur : %f, %f, %f;",&V1_temp ,&V2_temp ,&V3_temp );
                        if (result == 3){
                                V1 = V1_temp;
                                V2 = V2_temp;
                                V3 = V3_temp;
                                std::cout<<"Reçu ! " << std::endl;
                        }*/
                        /*int result = sscanf(message, "Vecteur : %f, %f, %f;",&V1_temp ,&V2_temp ,&V3_temp );
                        if (result == 3){
                                V1 = V1_temp;
                                V2 = V2_temp;
                                V3 = V3_temp;
                                std::cout<<"Reçu ! " << std::endl;
                        }*/
                        
                        if (result == 2){
				//if (angle_consigne_temp != 0)
                                angle_consigne = convertirAngle(angle_consigne_temp);
                                std::cout<<"Reçu ! " << angle_consigne << std::endl;
                        }
                
                        else {
                                std::cout<<"Erreur de format : "<< message << std::endl;
                        }
                } 
        
        }
}

///////////////////////

void * calcul_consigne_translation(void *a){
	//printf("Entré dans la fonction calcul consigne translation\n");

	float Omega_0 = V_Consigne_Globale/R_ROUES;	
	printf("Omega-trans : %f , Omega-rot : %f \n");
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	moteur1.vit_consigne = 0.0;
	moteur2.vit_consigne =  Omega_0;
	moteur3.vit_consigne = -Omega_0;
	return a;
}

void * calcul_consigne_rotation(void *a){
	//printf("Entré dans la fonction calcul consigne rotation\n");
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	
	moteur1.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference; 
	moteur2.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference;
	moteur3.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference;
	
	return a;
}


/*void * calcul_consigne_rotation_plus_translation(void *a){
	//printf("Entré dans la fonction calcul consigne rotation\n");
	
	float Omega_0 = V_Consigne_Globale/R_ROUES;	
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	moteur1.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 - Omega_0/4 + 2.5 * 0.01f * deplacement_consigne; 
	moteur2.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 + 2.5 * 0.01f * deplacement_consigne;
	moteur3.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 + Omega_0/4 - 2.5 * 0.01f * deplacement_consigne;

	return a;
}*/

void * actualiser_consigne(void *a){
	
	while(1){
		calcul_consigne_rotation(a);
		//printf("vit consigne : %f,%f,%f : \n", moteur1.vit_consigne,moteur1.vit_consigne,moteur1.vit_consigne);
		float pas_temps = 30 * 1e-3;
		float f = 2;//0.4;
		float alpha = pas_temps / (1/(2*PI*f)+pas_temps);
		moteur1.vit_consigne = alpha * moteur1.vit_consigne + (1-alpha) * moteur1.vit_consigne_prec;
		moteur1.vit_consigne_prec = moteur1.vit_consigne;
		moteur2.vit_consigne_prec = moteur2.vit_consigne;
		moteur3.vit_consigne_prec = moteur3.vit_consigne;
		usleep(30);
	}
	
	return a;
}


