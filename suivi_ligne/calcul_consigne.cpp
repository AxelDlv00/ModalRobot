#include "moteur.h"
#include "main.h"
#include "envois_consignes_moteurs.h"
#include "calcul_consigne.h"

extern float V_Consigne_Globale;
extern float angle_consigne;
extern float deplacement_consigne;
float angle_reference = 3.14/3;

void * calcul_consigne_translation(void *a){
		
	printf("Entré dans la fonction calcul consigne translation\n");

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
	/*
	moteur1.vit_consigne = OMEGA_ROT; 
	moteur2.vit_consigne = OMEGA_ROT;
	moteur3.vit_consigne = OMEGA_ROT;
	*/
	return a;
}


void * calcul_consigne_rotation_plus_translation(void *a){
	//printf("Entré dans la fonction calcul consigne rotation\n");
	
	float Omega_0 = V_Consigne_Globale/R_ROUES;	
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	moteur1.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 - Omega_0/4 + 2.5 * 0.01f * deplacement_consigne; 
	moteur2.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 + 2.5 * 0.01f * deplacement_consigne;
	moteur3.vit_consigne = angle_consigne * OMEGA_ROT / angle_reference * 0.55 + Omega_0/4 - 2.5 * 0.01f * deplacement_consigne;

	// moteur1.vit_consigne = -0.1f * deplacement_consigne; 
	// moteur2.vit_consigne = -2 * 0.1f * deplacement_consigne;
	// moteur3.vit_consigne = -0.1f * deplacement_consigne;

	// printf("vit_consigne : %f, %f, %f \n", moteur1.vit_consigne,moteur2.vit_consigne,moteur3.vit_consigne);

	//moteur1.vit_consigne = 0;
	//moteur2.vit_consigne = 0;
	//moteur3.vit_consigne = 0;


	
	return a;
}

void * actualiser_consigne(void *a){
	
	while(1){
		calcul_consigne_rotation_plus_translation(a);
		//calcul_consigne_rotation(a);
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
