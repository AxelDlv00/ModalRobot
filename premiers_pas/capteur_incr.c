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
//#include <softServo.h>

// 5=>21, 6=>22, 12=>26, 16=>27, 17=>0, 18=>1, 19=>24
 // LED Pin - wiringPi pin 0 is BCM_GPIO 17.
#define	LED	21

#define BUTTON 22

#define SERVO 26
#define RANGE 500 //en microsecondes

#define DIR 27
#define EN 0

#define SA 1
#define SB 24


// Variables globales

// Booléens
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;

// Gestion vitesse
int N = 1500;
int sensrot = 0;
int vit_mot = 0;
int vit_abs = 50;

int compteur = 0;

int nb_tour = 0;
int nb_tour_max = 4;

void myInterruptSA (void) { 
	int test_SB = digitalRead(SB);
	if (test_SB){
		compteur++ ; 
	}
	else {
		compteur-- ; 
	}
}

void init(){
	// INITIALISATION
	wiringPiSetup ();
	
	// DIRECTION
	pinMode (LED, OUTPUT) ;
	pinMode (BUTTON, INPUT) ;
	pinMode (SERVO, OUTPUT) ;
	
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
		nb_tour = compteur/360;
		if (abs(nb_tour) < nb_tour_max){
			if (allume == HIGH){
				vit_mot = +vit_abs;
			}
			else{
				vit_mot = -vit_abs;
			}
			if (vit_mot>=0){
				digitalWrite (DIR, 0);
			}
			else{
				digitalWrite (DIR, 1);
			}
			
		}
		else {
			vit_mot = 0;
		}
		softPwmWrite (EN, abs(vit_mot)) ;
	}
}

int main(int argc, char **argv)
{	
	init();
                                                                                  
	pthread_t t; // création du thread
	printf("Thread créé\n");
	
	pthread_create(&t,NULL,fait_tourner_moteur, NULL); // lancement programme servomoteur	
	
	// Actualisation de l'état du bouton (data6)
	while(1) {
		//Lecture
		estentraindappuyer = digitalRead(BUTTON); //HIGH ou LOW
		//usleep(100);
		
		allume = digitalRead(LED);
		
		
		//printf("allumé : %i , deja activé : %i \n",allume, dejaactive);
		printf("Compteur : %i\n", compteur);
		
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
	
	
	// pthread_join(t,NULL);
	return 0 ;
}

