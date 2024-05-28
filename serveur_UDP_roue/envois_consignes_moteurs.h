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
#include <signal.h>
#include <poll.h>

// RECEPTION DES DONNEES DE CONSIGNE
#include "recup_consigne.h"
#include "moteur.h"


#define PI 3.1415

// 5=>21, 6=>22, 12=>26, 16=>27, 17=>0, 18=>1, 19=>24
// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
#define	LED	5

#define BUTTON 6

// Moteur1
#define DIR1 19	 // vert 	 
#define EN1 26  // orange
#define SA1 27   // jaune
#define SB1 22   // blanc
#define KP1 1.   
#define KD1 2./ 3. 


// Moteur2
#define DIR2 6		 
#define EN2 13
#define SA2 23
#define SB2 24
#define KP2 1.
#define KD2 2./ 3.


// Moteur3
#define DIR3 12	 
#define EN3 16
#define SA3 20
#define SB3 21
#define KP3 1.
#define KD3 2./ 3.


#define K_red 360
#define K_P 1.0

#define LOW 0
#define HIGH 1

#define VIT_MAX 18600 //en rad/kilosecondes
#define RANGE 100 //100en microsecondes
#define N_moy 2
#define R_ROUES 2.7
#define OMEGA_ROT 2.*3.1415

extern Moteur moteur1, moteur2, moteur3;
extern Moteur* Liste_Moteur[3];

void init();
void * fait_tourner_moteur(void * moteur);
void * calcul_err (void *moteur);
void handler(int n);











