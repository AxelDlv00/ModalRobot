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


// Moteur1
#define DIR1 16		 
#define EN1 17
#define SA1 18
#define SB1 19

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

typedef struct moteur{
	int DIR, EN, SA, SB;
	
	float compteur_rad;
	
	float vit_consigne;
	float vit_reelle;
	float erreur ;
	float vit_envoyee;
	int vit_envoyee_radkilo;
	
	float cpt_t1, cpt_t2;
	} Moteur;
	
Moteur moteur1; 
float dt = 10000;


void myInterruptSA (Moteur *moteur) {  // codeur incrémental : compte le nombre d'incréments et donc de radians
	int test_SB = digitalRead(moteur->SB);
	if (test_SB){
		moteur->compteur_rad += PI/180 ; 
	}
	else {
		moteur->compteur_rad -= PI/180 ; 
	}
}

// Compteur 1
void myInterruptSA1 (void) {
	myInterruptSA(&moteur1);
}

void * calcul_err (void *moteur) {
	printf("Entré dans la fonction calcul erreur");
	Moteur *m = (Moteur *) moteur;
	while(1) {
		m->cpt_t1 = m->compteur_rad;
		usleep(dt);
		m->cpt_t2 = m->compteur_rad;
		
		m->vit_reelle = (m->cpt_t2-m->cpt_t1)/(dt/1000000);
		m->erreur = m->vit_consigne-m->vit_reelle;
	}
}

void init(){
	// INITIALISATION
	wiringPiSetupGpio ();
	
	moteur1.DIR = DIR1;
	moteur1.EN = EN1;
	moteur1.SA = SA1;
	moteur1.SB = SB1;
	moteur1.compteur_rad = 0;
	moteur1.vit_consigne = 3.1415;
	moteur1.vit_reelle = 0.;
	moteur1.erreur = 0.;
	moteur1.vit_envoyee = 0.;
	moteur1.vit_envoyee_radkilo = 0;
	
	// DIRECTION
	pinMode (LED, OUTPUT) ;
	pinMode (BUTTON, INPUT) ;
	
	wiringPiISR (moteur1.SA, INT_EDGE_FALLING, &myInterruptSA1) ;
	
	
	// ETATS INITIAUX
	digitalWrite (LED, LOW) ;	// Off
	digitalWrite (moteur1.EN, 0) ;	// Off
	digitalWrite (moteur1.DIR, 0) ;	// Sens trigo = 0, sens horaire = 1
	
	// INITIALISATION 
	softPwmCreate (moteur1.EN, 0, RANGE) ;
	
	printf("Fin d'initialisation\n");  
}



void * fait_tourner_moteur(void * moteur){
		
	printf("Entré dans la fonction moteur");
	
	Moteur *m = (Moteur *) moteur;
	while(1) {		
		// CHOIX ENVOIS
		if (allume == HIGH){
			m->vit_envoyee = m->vit_consigne + K_P*m->erreur;
		}
		else{
			m->vit_envoyee = 0.;
		}
		
		//printf("Vitesse envoyée : %f\n", vit_envoyee);
		
		m->vit_envoyee_radkilo = (int) (fabs(m->vit_envoyee)*1000);
		
		//ENVOIS
		softPwmWrite (m->EN, m->vit_envoyee_radkilo*RANGE/VIT_MAX) ;
		printf("Vitesse  : %i\n",m->vit_envoyee_radkilo*RANGE/VIT_MAX);
		
		if (m->vit_envoyee >0){
			digitalWrite (m->DIR, 0);
		}
		else{
			digitalWrite (m->DIR, 1);
		}
		usleep(10000);
	}
}

int main(int argc, char **argv)
{	
	init();
                                                                                  
	pthread_t t; // création du thread
	printf("Thread créé\n");
	
	pthread_create(&t,NULL,fait_tourner_moteur, &moteur1); // lancement programme servomoteur	
	pthread_create(&t,NULL,calcul_err, &moteur1); // lancement programme err
	
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

