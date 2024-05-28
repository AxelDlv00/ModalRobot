#include <pthread.h>
#include <cstdio>
#include "main.h"

// Variables globles
char buffer[BUFFER_SIZE];

float angle_consigne = 0.;
int test_avancer = 0;
float dB[6];

int main(){
	pthread_t t_serveur_python;
	pthread_t t_client_udp;
	pthread_t t_calcul_consigne;
	
	//pthread_t t_micro;
	
	printf("Thread créés\n");
	
	pthread_create(&t_serveur_python, NULL, serveur, NULL);

	pthread_create(&t_calcul_consigne, NULL, recup_consigne, NULL);
	pthread_create(&t_client_udp, NULL, client, NULL);
	
	pthread_join(t_serveur_python, NULL);
	pthread_join(t_client_udp, NULL);	
	pthread_join(t_calcul_consigne, NULL);
}
