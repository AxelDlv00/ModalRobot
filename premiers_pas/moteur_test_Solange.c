
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


// Variables globales
#define N1 2000
#define N2 1000

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

// Liste des fd nécessaires
int fd5, fd6, fd12, fd; 



void init(){
	// INITIALISATION
	
	// Initialisation chaines à envoyer
	char dout[] = "out";
	char din[] = "in";
	char dstring[100]; //prend des char dynamiquement

	// Initialisation moteur (pin12)
		// pour créer gpio12
	sprintf(dstring,"%d", 12);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,2); // /!\ 12 c'est 2 caractères
	close(fd);

		// pour envoyer des données
	fd12 = open("/sys/class/gpio/gpio12/direction",O_WRONLY);
	write(fd,dout,3);
	close(fd);
	
	// Initialisation bouton (pin5 et pin6)
	
		// pour créer gpio5
	sprintf(dstring,"%d",5);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,1);
	close(fd);
	
		// pour envoyer allumage/extinction
	fd = open("/sys/class/gpio/gpio5/direction",O_WRONLY);
	write(fd,dout,3);
	close(fd);
	
		// pour créer gpio6
	sprintf(dstring,"%d",6);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,1);
	close(fd);

		// pour reçevoir l'état d'appui
	fd6 = open("/sys/class/gpio/gpio6/direction",O_WRONLY);
	write(fd6,din,2);
	close(fd);
	
	// Initialisation état
	fd5 = open("/sys/class/gpio/gpio5/value",O_WRONLY); //Initialisation à éteint
	data5[0] = 48;
	read(fd5,data5,1);
	close(fd5);
	
	printf("Fin d'initialisation\n");  
}

void * fait_tourner_servomoteur(void * a){
	float* tt = (float*) a;
		
	printf("Entré dans la fonction moteur");
	
	while(1) {
		if (allume){
			fd12 = open("/sys/class/gpio/gpio12/value",O_WRONLY);
			data12[0] = 49;//1
			write(fd12,data12,1);
			usleep(N);
			data12[0] = 48;//0
			write(fd12,data12,1);
			usleep(20000-N); 
			close(fd12);
		}
		else{
			fd12 = open("/sys/class/gpio/gpio12/value",O_WRONLY);
			data12[0] = 49;//1
			write(fd12,data12,1);
			usleep(0);
			data12[0] = 48;//0
			write(fd12,data12,1);
			usleep(20000); 
			close(fd12);
		}
	}
}

int main(int argc, char **argv)
{	
	init();
                                                                                  
	pthread_t t; // création du thread
	printf("Thread créé\n");
	
	float testchiffre = 42.;
	pthread_create(&t,NULL,fait_tourner_servomoteur,&testchiffre); // lancement programme servomoteur	
	
	// Actualisation de l'état du bouton (data6)
	while(1) {
		fd6 = open("/sys/class/gpio/gpio6/value",O_RDONLY);
		read(fd6,data6,1);
		close(fd6);
		usleep(100);
		
		fd5 = open("/sys/class/gpio/gpio5/value",O_RDWR); //Lit lumière
		read(fd5,data5,1);
		
		estentraindappuyer = (1-data6[0]+48); // 1 si appuyé 0 sinon
		
		printf("allumé : %i , deja activé : %i \n",allume, dejaactive);
		
		if (estentraindappuyer&&(!dejaactive)) { //appuyé
			if (allume){
				data5[0]= 48; //Eteindre
				allume = 0;
			}
			else {
				data5[0]= 49;//Allumer
				allume = 1;
			}
			//printf("data%i %i \n",6,data6[0]);
			//printf("data%i %i \n",5,data5[0]);
			
			dejaactive = 1;
		}
		else if (dejaactive&&(!estentraindappuyer)) {
			dejaactive = 0;
		}
		
		
		write(fd5,data5,1);
		close(fd5);
	}
	
	
	// pthread_join(t,NULL);
	return 0 ;
}


                                   
