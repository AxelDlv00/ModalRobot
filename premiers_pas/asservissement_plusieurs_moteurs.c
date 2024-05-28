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
#define	LED	5

#define BUTTON 6

// Moteur1
#define DIR1 16		 
#define EN1 17
#define SA1 18
#define SB1 19
#define KP1 1.0


// Moteur2
#define DIR2 12		 
#define EN2 13
#define SA2 22
#define SB2 23
#define KP2 1.0


// Moteur3
#define DIR3 24		 
#define EN3 25
#define SA3 26
#define SB3 27
#define KP3 1.0

#define K_red 360
#define K_P 1.0

#define VIT_MAX 18600 //en rad/kilosecondes
#define RANGE 100 //en microsecondes

// Variables globales

// Booléens du bouton-lumière
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;

typedef struct moteur{
	
	int id;
	
	int DIR, EN, SA, SB;
	
	float compteur_rad;
	
	float vit_consigne;
	float vit_reelle;
	float erreur ;
	float vit_envoyee;
	int vit_envoyee_radkilo;
	
	float Kp;
	
	float cpt_t1, cpt_t2;
	} Moteur;
Moteur moteur1, moteur2, moteur3, moteuri;
Moteur* Liste_Moteur[3] = {&moteur1, &moteur2, &moteur3};

float dt = 10000;

/* COMPTEUR */
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

// Compteur 2
void myInterruptSA2 (void) {
	myInterruptSA(&moteur2);
}
// Compteur 3
void myInterruptSA3 (void) {
	myInterruptSA(&moteur3);
}


void * calcul_err (void *moteur) {
	printf("Entré dans la fonction calcul erreur\n");
	Moteur *m = (Moteur *) moteur;
	while(1) {
		m->cpt_t1 = m->compteur_rad;
		usleep(dt);
		m->cpt_t2 = m->compteur_rad;
		
		m->vit_reelle = (m->cpt_t2-m->cpt_t1)/(dt/1000000);
		printf("Vitesse mesurée moteur %i : %f\n", m->id, m->vit_reelle);
		m->erreur = m->vit_consigne-m->vit_reelle;
	}
}

void init(){
	// INITIALISATION
	wiringPiSetupGpio ();
	
	moteur1.id = 1;
	moteur1.DIR = DIR1;
	moteur1.EN = EN1;
	moteur1.SA = SA1;
	moteur1.SB = SB1;
	moteur1.Kp = KP1;
	
	moteur1.compteur_rad = 0;
	moteur1.vit_consigne = 3.1415;
	moteur1.vit_reelle = 0.;
	moteur1.erreur = 0.;
	moteur1.vit_envoyee = 0.;
	moteur1.vit_envoyee_radkilo = 0;
	
	moteur2.id = 2;
	moteur2.DIR = DIR2;
	moteur2.EN = EN2;
	moteur2.SA = SA2;
	moteur2.SB = SB2;
	moteur2.Kp = KP2;
	
	moteur2.compteur_rad = 0;
	moteur2.vit_consigne = 3.1415;
	moteur2.vit_reelle = 0.;
	moteur2.erreur = 0.;
	moteur2.vit_envoyee = 0.;
	moteur2.vit_envoyee_radkilo = 0;

	moteur3.id = 3;
	moteur3.DIR = DIR3;
	moteur3.EN = EN3;
	moteur3.SA = SA3;
	moteur3.SB = SB3;
	moteur3.Kp = KP3;
	
	moteur3.compteur_rad = 0;
	moteur3.vit_consigne = 3.1415;
	moteur3.vit_reelle = 0.;
	moteur3.erreur = 0.;
	moteur3.vit_envoyee = 0.;
	moteur3.vit_envoyee_radkilo = 0;
	
	pinMode (LED, OUTPUT) ;
	pinMode (BUTTON, INPUT) ;
	
	for (int i=0; i<3; i++){
		moteuri = *Liste_Moteur[i];
		pinMode (moteuri.DIR, OUTPUT) ;
		pinMode (moteuri.EN, OUTPUT) ;
		pinMode (moteuri.SA, INPUT) ;
		pinMode (moteuri.SB, INPUT) ;
		}
	wiringPiISR (moteur1.SA, INT_EDGE_FALLING, &myInterruptSA1) ;
	wiringPiISR (moteur2.SA, INT_EDGE_FALLING, &myInterruptSA2) ;
	wiringPiISR (moteur3.SA, INT_EDGE_FALLING, &myInterruptSA3) ;	
	
	// ETATS INITIAUX
	digitalWrite (LED, LOW) ;	// Off
	for (int i=0; i<3; i++){
		moteuri = *Liste_Moteur[i];
		digitalWrite (moteuri.EN, 0) ;	// Off
		digitalWrite (moteuri.DIR, 0) ;	// Sens trigo = 0, sens horaire = 1
		softPwmCreate (moteuri.EN, 0, RANGE) ;
		}
	printf("Fin d'initialisation\n");  
}


void * fait_tourner_moteur(void * moteur){
		
	printf("Entré dans la fonction moteur\n");
	
	Moteur *m = (Moteur *) moteur;
	while(1) {		
		// CHOIX ENVOIS
		if (allume == HIGH){
			m->vit_envoyee = m->vit_consigne + (m->Kp)*(m->erreur);
		}
		else{
			m->vit_envoyee = 0.;
		}
		
		//printf("Vitesse envoyée moteur %i : %f\n", m->id, m->vit_envoyee);
		
		m->vit_envoyee_radkilo = (int) (fabs(m->vit_envoyee)*1000);
		
		//ENVOIS
		softPwmWrite (m->EN, m->vit_envoyee_radkilo*RANGE/VIT_MAX) ;
		//printf("Vitesse  : %i\n",m->vit_envoyee_radkilo*RANGE/VIT_MAX);
		
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
                                                                                  
	pthread_t t[3]; // création du thread
	printf("Thread créé\n");
	
	pthread_create(&t[0],NULL,fait_tourner_moteur, &moteur1); // lancement programme servomoteur	
	pthread_create(&t[0],NULL,calcul_err, &moteur1); // lancement programme err
	pthread_create(&t[1],NULL,fait_tourner_moteur, &moteur2); // lancement programme servomoteur	
	pthread_create(&t[1],NULL,calcul_err, &moteur2); // lancement programme err
	pthread_create(&t[2],NULL,fait_tourner_moteur, &moteur3); // lancement programme servomoteur	
	pthread_create(&t[2],NULL,calcul_err, &moteur3); // lancement programme err 
	
	
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
	
	for (int i=0; i<3; i++){
		pthread_join(t[i],NULL);
	}
	
	return 0 ;
}

