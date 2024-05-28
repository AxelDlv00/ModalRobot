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

// 5=>21, 6=>22, 12=>26
// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
#define	LED	21
#define BUTTON 22
#define SERVO 26
#define RANGE 200

// Variables globales
char data5[2];
char data6[2];
char data12[2];

// Booléens
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;

// Gestion vitesse
int N = 1500;

void init(){
	// INITIALISATION
	wiringPiSetup ();
	
	// DIRECTION
	pinMode (LED, OUTPUT) ;
	pinMode (BUTTON, INPUT) ;
	pinMode (SERVO, OUTPUT) ;
	
	// ETATS INITIAUX
	digitalWrite (LED, LOW) ;	// Off
	
	// INITIALISATION SERVO
	softPwmCreate (SERVO, 0, RANGE) ;
	//softServoSetup (SERVO, 1, 2, 3, 4, 5, 6, 7) ;
	
	printf("Fin d'initialisation\n");  
}

void * fait_tourner_servomoteur(void * a){
		
	printf("Entré dans la fonction moteur");
	
	while(1) {
		if (allume == HIGH){
			softPwmWrite (SERVO, 10) ;
			//softServoWrite (SERVO,  1500) ;
		}
		else{
			softPwmWrite (SERVO, 30);
		}
	}
}

int main(int argc, char **argv)
{	
	init();
                                                                                  
	pthread_t t; // création du thread
	printf("Thread créé\n");
	
	pthread_create(&t,NULL,fait_tourner_servomoteur, NULL); // lancement programme servomoteur	
	
	// Actualisation de l'état du bouton (data6)
	while(1) {
		//Lecture
		estentraindappuyer = digitalRead(BUTTON); //HIGH ou LOW
		//usleep(100);
		
		allume = digitalRead(LED);
		
		
		printf("allumé : %i , deja activé : %i \n",allume, dejaactive);
		
		if ((estentraindappuyer==HIGH) && (!dejaactive)) { //appuyé
			if (allume){
				digitalWrite (LED, LOW) ; //Eteindre
				allume = LOW;
			}
			else {
				digitalWrite (LED, HIGH) ;//Allumer
				allume = HIGH;
			}
			//printf("data%i %i \n",6,data6[0]);
			//printf("data%i %i \n",5,data5[0]);
			
			dejaactive = 1;
		}
		else if (dejaactive && (estentraindappuyer==LOW)) {
			dejaactive = 0;
		}

	}
	
	
	// pthread_join(t,NULL);
	return 0 ;
}
