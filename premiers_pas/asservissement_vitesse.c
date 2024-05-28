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
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
//#include <softServo.h>

#define PI 3.1415

// 5=>21, 6=>22, 12=>26, 16=>27, 17=>0, 18=>1, 19=>24
 // LED Pin - wiringPi pin 0 is BCM_GPIO 17.
#define	LED	21

#define BUTTON 22

#define SERVO 26


#define DIR 16
#define EN 17
#define SA 18
#define SB 19

#define K_red 360
#define K_P 0.0

#define VIT_MAX 18600 //en rad/kilosecondes
#define RANGE 100 //en microsecondes

// Variables globales

// Booléens
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;

// Gestion vitesse (rad/s)
float vit_consigne = 3.1415;
float vit_reelle = 0.;
float erreur = 0.;
float vit_envoyee = 0.;
int vit_envoyee_radkilo = 0;

float compteur_rad = 0.;

int nb_tour = 0;

float cpt_t1, cpt_t2;
float dt = 10000;


void myInterruptSA (void) {  // codeur incrémental : compte le nombre d'incréments et donc de radians
	int test_SB = digitalRead(SB);
	if (test_SB){
		compteur_rad += PI/180 ; 
	}
	else {
		compteur_rad -= PI/180 ; 
	}
}

void * calcul_err (void *a) {
	printf("Entré dans la fonction calcul erreur");
	
	while(1) {
		cpt_t1 = compteur_rad;
		usleep(dt);
		cpt_t2 = compteur_rad;
		
		vit_reelle = (cpt_t2-cpt_t1)/(dt/1000000);
		printf("Cpt-t1 : %f, Cpt-t2 : %f\n", cpt_t1, cpt_t2);
		printf("Vitesse en rad/s : %f\n", vit_reelle);
		erreur = vit_consigne-vit_reelle;
	}
}

void init(){
	// INITIALISATION
	wiringPiSetup ();
	
	// DIRECTION
	pinMode (LED, OUTPUT) ;
	pinMode (BUTTON, INPUT) ;
	//pinMode (SERVO, OUTPUT) ;
	
	pinMode (DIR, OUTPUT) ;
	pinMode (EN, OUTPUT) ;
	
	pinMode (SA, INPUT) ;
	pinMode (SB, INPUT) ;
	
	wiringPiISR (SA, INT_EDGE_FALLING, &myInterruptSA) ;
	
	
	// ETATS INITIAUX
	digitalWrite (LED, LOW) ;	// Off
	digitalWrite (EN, 0) ;	// Off
	digitalWrite (DIR, 0) ;	// Sens trigo = 0, sens horaire = 1
	
	// INITIALISATION 
	softPwmCreate (EN, 0, RANGE) ;
	
	printf("Fin d'initialisation\n");  
}



void * fait_tourner_moteur(void * a){
		
	printf("Entré dans la fonction moteur");
	
	while(1) {
		nb_tour = compteur_rad/(2*PI);
		
		// CHOIX ENVOIS
		if (allume == HIGH){
			vit_envoyee = vit_consigne + K_P*erreur;
		}
		else{
			vit_envoyee = 0.;
		}
		
		//printf("Vitesse envoyée : %f\n", vit_envoyee);
		
		vit_envoyee_radkilo = (int) (fabs(vit_envoyee)*1000);
		
		//ENVOIS
		softPwmWrite (EN, vit_envoyee_radkilo*RANGE/VIT_MAX) ;
		printf("Vitesse  : %i\n",vit_envoyee_radkilo*RANGE/VIT_MAX);
		
		if (vit_envoyee >0){
			digitalWrite (DIR, 0);
		}
		else{
			digitalWrite (DIR, 1);
		}
		usleep(10000);
	}
}

int main(int argc, char **argv)
{	
	init();
                                                                                  
	pthread_t t; // création du thread
	printf("Thread créé\n");
	
	pthread_create(&t,NULL,fait_tourner_moteur, NULL); // lancement programme servomoteur	
	pthread_create(&t,NULL,calcul_err, NULL); // lancement programme err
	
	// Actualisation de l'état du bouton (data6)
	while(1) {
		//Lecture
		estentraindappuyer = digitalRead(BUTTON); //HIGH ou LOW
		//usleep(100);
		
		allume = digitalRead(LED);
		
		
		//printf("allumé : %i , deja activé : %i \n",allume, dejaactive);
		//printf("Compteur : %f\n", compteur_rad);
		
		if ((estentraindappuyer==HIGH) && (!dejaactive)) { //appuyé
			if (allume){
				digitalWrite (LED, LOW) ; //Eteindre
				allume = LOW;
			}
			else {
				digitalWrite (LED, HIGH) ;//Allumer
				allume = HIGH;
			}
			
			dejaactive = 1;
		}
		else if (dejaactive && (estentraindappuyer==LOW)) {
			dejaactive = 0;
		}

	}
	
	
	pthread_join(t,NULL);
	return 0 ;
}

