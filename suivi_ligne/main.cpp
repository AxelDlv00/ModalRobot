#include "moteur.h"
#include "envois_consignes_moteurs.h"
#include "calcul_consigne.h"
#include "calcul_angle_beta.h"
#include "myWiringPi.h"
#include "main.h"

// Variables globles
float V_Consigne_Globale = 14.0; 
int running = 1;
float dt = 10000.* 0.4;
// Booléens du bouton-lumière
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;
float angle_consigne = 0.;
float deplacement_consigne = 0.;

int main(int argc, char **argv)
{	
	init();
	signal(SIGINT,handler);
                                                                                  
	pthread_t t[3]; // création du thread
	pthread_t t_beta;
	pthread_t t_consigne;
	printf("Thread créé\n");
	
	//calcul_consigne_translation(NULL);
	calcul_consigne_rotation(NULL);

    pthread_attr_t attr[2];
    int priorities[2] = {80, 80}; 

    for (int i = 0; i < 2; i++) {
        pthread_attr_init(&attr[i]);
        struct sched_param param;
        param.sched_priority = priorities[i];
        pthread_attr_setschedparam(&attr[i], &param);
    }
	
	pthread_create(&t[0], &attr[0],fait_tourner_moteur, &moteur1); // lancement programme servomoteur	
	pthread_create(&t[0], &attr[1],calcul_err, &moteur1); // lancement programme err
	pthread_create(&t[1], &attr[0],fait_tourner_moteur, &moteur2); // lancement programme servomoteur	
	pthread_create(&t[1], &attr[1],calcul_err, &moteur2); // lancement programme err
	pthread_create(&t[2], &attr[0],fait_tourner_moteur, &moteur3); // lancement programme servomoteur	
	pthread_create(&t[2], &attr[1],calcul_err, &moteur3); // lancement programme err 
	pthread_create(&t_beta, &attr[0],calcul_angle_beta, NULL);
	pthread_create(&t_beta, &attr[0],actualiser_consigne, NULL);
	
	// Actualisation de l'état du bouton (data6)
	while(1) {
		//Lecture
		estentraindappuyer = mydigitalRead(BUTTON); //HIGH ou LOW
		usleep(100);
		
		allume = mydigitalRead(LED);
		usleep(100);
		
		//printf("allumé : %i , deja activé : %i \n",allume, dejaactive);
		//printf("Compteur : %ld\n", moteur1.cpt);
		
		
		
		if ((estentraindappuyer==HIGH) && (!dejaactive)) { //appuyé
			if (allume){
				mydigitalWrite (LED, LOW) ; //Eteindre
				allume = LOW;
			}
			else {
				mydigitalWrite (LED, HIGH) ;//Allumer
				allume = HIGH;
			}
			
			dejaactive = 1;
		}
		else if (dejaactive && (estentraindappuyer==LOW)) {
			dejaactive = 0;
		}

	}
	
	for (int i=0; i<3; i++){
		pthread_join(t[i],NULL);
	}
	pthread_join(t_beta,NULL);
	pthread_join(t_consigne,NULL);
	
	return 0 ;
}

