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
//#include <wiringPi.h>
//#include <softPwm.h>
#include <math.h>
//#include <softServo.h>
#include <signal.h>

#include <poll.h>

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
#define KP1 5.


// Moteur2
#define DIR2 12		 
#define EN2 13
#define SA2 22
#define SB2 23
#define KP2 4.


// Moteur3
#define DIR3 24		 
#define EN3 25
#define SA3 26
#define SB3 27
#define KP3 4.

#define K_red 360
#define K_P 1.0

#define LOW 0
#define HIGH 1

#define VIT_MAX 18600 //en rad/kilosecondes
#define RANGE 100 //100en microsecondes
#define N_moy 5

#define R_ROUES 2.7
#define OMEGA_ROT 3.*3.1415

// Variables globales

float V_Consigne_Globale = 2.*14.0; 
int running = 1;
float dt = 10000.*0.2;
// Booléens du bouton-lumière
int etat = 0;
int dejaactive = 0; 
int estentraindappuyer = 0;
int allume = 0;
pthread_mutex_t	m= PTHREAD_MUTEX_INITIALIZER;

int mydigitalRead(int n){
	char path[100];
	char buf[]={'a','s'};
	sprintf(path,"/sys/class/gpio/gpio%d/value",n+512);
	//pthread_mutex_lock(&m);
	int fd = open(path,O_RDONLY);
	if (fd == -1) {perror("open failed dig read");};
	int res;
	res = read(fd,buf,1);
	if(res<0) {
		perror("Prob");
		//pthread_mutex_unlock(&m);
		exit(-1);
		}
	else{
		
		close(fd);
		//pthread_mutex_unlock(&m);
		//printf("path (%d) : %s buf %i : %c\n",res, path, n+512, buf[0]);
		if(buf[0]=='1')
			return 1;
		else 
			return 0;
	}
}
void mydigitalWrite(int n,int val){
	char path[100];
	char buf[]="0";
	sprintf(buf,"%d",val);
	sprintf(path,"/sys/class/gpio/gpio%d/value",n+512);
	//pthread_mutex_lock(&m);
	int fd = open(path,O_WRONLY);
	if (fd == -1) {perror("open failed dig write");};
	write(fd,buf,2);
	usleep(100);
	close(fd);
	//pthread_mutex_unlock(&m);
	
}
int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))

    {
        fclose(file);
        return 1;
    }
    return 0;
}

void exportPin(int n){
	char code[100], code1[100];
	;
	char buf[2];
	sprintf(code, "/sys/class/gpio/gpio%d/value", n +512);
	sprintf(code1,"echo %d > /sys/class/gpio/export", n +512);

	if (exists (code) == 0){
	// Creation if doesn't exists
	system(code1);
	usleep(100);
	
	printf("%s\n",code1);
	}		
	else {
		printf("%d already exported !\n",n+512);
	}
	
}
void unexportPin(int n){
	char code[100], code1[100];
	;
	char buf[2];
		sprintf(code,"echo %d > /sys/class/gpio/export", n +512);
	system(code);
	usleep(100);
	printf("%s\n",code);
	
}
void pinModeOut(int n){
	char code[100], code1[100];
	;
	char buf[2];
	sprintf(code, "/sys/class/gpio/gpio%d/direction", n +512);
	while (exists (code) == 0){
		usleep(10);
	}
	sprintf(code1,"echo out > /sys/class/gpio/gpio%d/direction", n +512);
	system(code1);
	usleep(100);
	printf("%s\n",code1);
	
}
void pinModeIn(int n){
	char code[100],code1[100];
	char buf[2];
	sprintf(code, "/sys/class/gpio/gpio%d/direction", n +512);
	while (exists (code) == 0){
		usleep(10);
	}
	sprintf(code1,"echo in > /sys/class/gpio/gpio%d/direction", n +512);
	system(code1);
	usleep(100);
	printf("%s\n",code1);
	
}


typedef struct moteur{
	
	int id;
	
	int DIR, EN, SA, SB;
	int SA_val1, SA_val0, SB_val1;
	
	float compteur_rad;
	
	float vit_consigne;
	float vit_reelle;
	float erreur ;
	float vit_envoyee;
	int vit_envoyee_radkilo;
	float pwm;
	float Kp;
	
	float cpt_t1, cpt_t2;
	long cpt;
	
	} Moteur;
Moteur moteur1, moteur2, moteur3, moteuri;
Moteur* Liste_Moteur[3] = {&moteur1, &moteur2, &moteur3};

void handler(int n){
	running=0;
	usleep(100);
	mydigitalWrite(moteur1.EN,0);
	mydigitalWrite(moteur2.EN,0);
	mydigitalWrite(moteur3.EN,0);
	printf("Fin de moteurs !\n");
	exit(0);

}

int mywaitForInterrupt (int pin, int mS){
	int fd, x ;
	unsigned int c ;
	struct pollfd polls ;
	char strfile[100];
	sprintf(strfile,"/sys/class/gpio/gpio%d/value",pin+512);
	fd = open(strfile,O_RDONLY);
	if (fd == -1) {perror("open failed dig mywaitForInterrupt");};

	// Setup poll structure

	  polls.fd     = fd ;
	  polls.events = POLLPRI | POLLERR ;

	// Wait for it ...

	  x = poll (&polls, 1, mS) ;

	// If no error, do a dummy read to clear the interrupt
	//	A one character read appars to be enough.

	  if (x > 0)
	  {
		lseek (fd, 0, SEEK_SET) ;	
// Rewind
		(void)read (fd, &c, 1) ;	// Read & clear
	  }
	  close(fd);

	  return x ;
}



/* COMPTEUR */
void *myInterruptSA (void *arg) 
{  // codeur incrémental : compte le nombre d'incréments et donc de radians
	Moteur *moteur=(Moteur*) arg;
	 struct sched_param param ;

	param.sched_priority = sched_get_priority_max (SCHED_RR) ;
	pthread_setschedparam (pthread_self (), SCHED_RR, &param);
	char strfileedg[100]; 
	sprintf(strfileedg,"/sys/class/gpio/gpio%d/edge",moteur->SA+512);
	FILE* fdEdge = fopen(strfileedg,"w");
	fprintf(fdEdge,"rising");
	fclose(fdEdge);
	moteur->cpt = 0;
	
	while(running){
		moteur->SA_val1 = moteur->SA_val0;
		
		if (mywaitForInterrupt(moteur->SA,-1)) {
			
			moteur->SA_val0 = mydigitalRead(moteur->SA);;
		
		
			if(moteur->SA_val0 >  moteur->SA_val1){ 
				int test_SB = mydigitalRead(moteur->SB);
				if (test_SB)
				{
					moteur->compteur_rad += PI/180. ;
					moteur->cpt +=1; 
				}
				else 
				{
					moteur->compteur_rad -= PI/180. ;
					moteur->cpt -=1; 
				}
			} 
		}
		else
		{printf("Err Timeout isr SA %d\n",moteur->cpt);
		}
	}
}

void *my_ThreadSoftPwm(void *arg)
{
	Moteur *moteur=(Moteur*) arg;
	int period = RANGE;
	int cpt = 0;
	struct sched_param param ;

    param.sched_priority = sched_get_priority_max (SCHED_RR) ;
    pthread_setschedparam (pthread_self (), SCHED_RR, &param) ;
	while(running)
	{				
			if(moteur->pwm > 0)
			{mydigitalWrite(moteur->EN,1);
				usleep((int)moteur->pwm);}
			//usleep((int)(RANGE));
			if(moteur->pwm == period)
			{
			mydigitalWrite(moteur->EN,0);
			usleep((int)(period - moteur->pwm));
			}					
	}
	
}

//int mysoftPwm(int en, float ratio);


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
	float v;
	while(1) {
		// On va faire une moyenne de la vitesse sur quelques itérations pour éviter les gros écarts à la vraie valeur
		v = 0.;
		for(int k=0;k<N_moy;k++){
			m->cpt_t1 = m->compteur_rad;
			usleep((int) dt);
			m->cpt_t2 = m->compteur_rad;
			//printf("moteur %i : cpt_t1 : %f et cpt_t2 : %f et cpt : %d\n", m->id, m->cpt_t1, m->cpt_t2, m->cpt);
			v += (m->cpt_t2-m->cpt_t1)/(dt/1000000);
		}
		v /= N_moy;
		m->vit_reelle = v;
		printf("Vitesse mesurée moteur %i : %f\n", m->id, m->vit_reelle);
		m->erreur = m->vit_consigne-m->vit_reelle;
	}
}

void init(){
	// INITIALISATION
//	wiringPiSetupGpio ();
	printf("init moteur 1\n");
	
	moteur1.id = 1;
	moteur1.DIR = DIR1;
	moteur1.EN = EN1;
	moteur1.SA = SA1;
	moteur1.SB = SB1;
	moteur1.Kp = KP1;
	
	moteur1.compteur_rad = 0;
	moteur1.vit_consigne = 0;
	moteur1.vit_reelle = 0.;
	moteur1.erreur = 0.;
	moteur1.vit_envoyee = 0.;
	moteur1.vit_envoyee_radkilo = 0;
	
	printf("init moteur 2\n");
	
	moteur2.id = 2;
	moteur2.DIR = DIR2;
	moteur2.EN = EN2;
	moteur2.SA = SA2;
	moteur2.SB = SB2;
	moteur2.Kp = KP2;
	
	moteur2.compteur_rad = 0;
	moteur2.vit_consigne = 0;
	moteur2.vit_reelle = 0.;
	moteur2.erreur = 0.;
	moteur2.vit_envoyee = 0.;
	moteur2.vit_envoyee_radkilo = 0;
	
	printf("init moteur 3\n");

	moteur3.id = 3;
	moteur3.DIR = DIR3;
	moteur3.EN = EN3;
	moteur3.SA = SA3;
	moteur3.SB = SB3;
	moteur3.Kp = KP3;
	
	moteur3.compteur_rad = 0;
	moteur3.vit_consigne = 0;
	moteur3.vit_reelle = 0.;
	moteur3.erreur = 0.;
	moteur3.vit_envoyee = 0.;
	moteur3.vit_envoyee_radkilo = 0;
	
	printf("init bouton\n");
	
	exportPin(LED);
	exportPin(BUTTON);
	
	pinModeOut (LED) ;
	pinModeIn (BUTTON) ;
	printf("exports bouton finis\n");

	for (int i=0; i<3; i++){
		moteuri = *Liste_Moteur[i];
		exportPin (moteuri.DIR);//, OUTPUT) ;
		exportPin (moteuri.EN);//, OUTPUT) ;
		exportPin (moteuri.SA);//, INPUT) ;
		exportPin (moteuri.SB);//, INPUT) ;
		}
	for (int i=0; i<3; i++){
		moteuri = *Liste_Moteur[i];
		pinModeOut (moteuri.DIR);//, OUTPUT) ;
		pinModeOut (moteuri.EN);//, OUTPUT) ;
		pinModeIn (moteuri.SA);//, INPUT) ;
		pinModeIn (moteuri.SB);//, INPUT) ;
		}
	
	pthread_t t[3];
    pthread_attr_t attr;
    int priorities = 99;

    for (int i = 0; i < 3; i++) {
        pthread_attr_init(&attr);
        struct sched_param param;
        param.sched_priority = priorities;
        pthread_attr_setschedparam(&attr, &param);
    }

	//wiringPiISR (moteur1.SA, INT_EDGE_FALLING, &myInterruptSA1) ;
	pthread_create(&t[0], &attr,myInterruptSA,&moteur1);
	//wiringPiISR (moteur2.SA, INT_EDGE_FALLING, &myInterruptSA2) ;
	pthread_create(&t[1], &attr,myInterruptSA,&moteur2);
	//wiringPiISR (moteur3.SA, INT_EDGE_FALLING, &myInterruptSA3) ;	
	pthread_create(&t[2], &attr,myInterruptSA,&moteur3);
	
	// ETATS INITIAUX
	mydigitalWrite (LED, LOW) ;	// Off
	pthread_t t_mot[3];

	for (int i=0; i<3; i++){
		moteuri = *Liste_Moteur[i];
		mydigitalWrite (moteuri.EN, 0) ;	// Off
		mydigitalWrite (moteuri.DIR, 0) ;	// Sens trigo = 0, sens horaire = 1
		//softPwmCreate (moteuri.EN, 0, RANGE) ;
		moteuri.pwm=0.0;
		pthread_create(&t_mot[i],&attr,my_ThreadSoftPwm,Liste_Moteur[i]);
		}
	printf("Fin d'initialisation\n");  
}


void * fait_tourner_moteur(void * moteur){
		
	printf("Entré dans la fonction moteur\n");
	
	Moteur *m = (Moteur *) moteur;
	while(running) {		
		// CHOIX ENVOIS
		printf("Vitesse consigne moteur %i : %f\n", m->id, m->vit_consigne);
		printf("Pwm moteur %i : %f\n", m->id, m->pwm);
		if (allume == HIGH){
			m->vit_envoyee = m->vit_consigne + (m->Kp)*(m->erreur);
			//printf("Erreur moteur %i : %f\n", m->id, m->erreur);
		}
		else{
			m->vit_envoyee = 0.;
		}
		
		//printf("Vitesse envoyée moteur %i : %f\n", m->id, m->vit_envoyee);
		
		m->vit_envoyee_radkilo = (int) (fabs(m->vit_envoyee)*1000);
		
		//ENVOIS
		//softPwmWrite (m->EN, m->vit_envoyee_radkilo*RANGE/VIT_MAX) ;
		
		//////////////////// A MODIFIER ////////////////////
		//m->pwm=VIT_MAX/3.;
		
		m->pwm=m->vit_envoyee_radkilo*RANGE/VIT_MAX;
		
		//printf("Vitesse  : %i\n",m->vit_envoyee_radkilo*RANGE/VIT_MAX);
		
		if (m->vit_envoyee >0){
			mydigitalWrite (m->DIR, 1);
		}
		else{
			mydigitalWrite (m->DIR, 0);
		}
		usleep(10000);
	}
}

void * calcul_consigne_translation(void *a){
		
	printf("Entré dans la fonction calcul consigne translation\n");

	float Omega_0 = V_Consigne_Globale/R_ROUES;	
	printf("Omega-trans : %f , Omega-rot : %f \n");
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	moteur1.vit_consigne = 0.0;
	moteur2.vit_consigne =  Omega_0;
	moteur3.vit_consigne = -Omega_0;
}

void * calcul_consigne_rotation(void *a){
		
	printf("Entré dans la fonction calcul consigne rotation\n");
	
	//CHANGEMENT DE VITESSE DE CONSIGNE
	moteur1.vit_consigne = OMEGA_ROT;
	moteur2.vit_consigne = OMEGA_ROT;
	moteur3.vit_consigne = OMEGA_ROT;
}

int main(int argc, char **argv)
{	
	init();
	signal(SIGINT,handler);
                                                                                  
	pthread_t t[3]; // création du thread
	printf("Thread créé\n");
	
	calcul_consigne_translation(NULL);
	//calcul_consigne_rotation(NULL);

    pthread_attr_t attr[2];
    int priorities[2] = {50, 50}; 

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
	
	return 0 ;
}

