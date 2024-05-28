#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define N1 2000
#define N2 1000


char data6[2];
int etat = 0;
char data[2];
int N = 1500;

int fd6, fd12, fd;


void * fait_tourner_servomoteur(void * a){
	float* tt = (float*) a;
	int state=0;
	printf("chiffre %f \n",*tt);
	while(1) {
		if ((1-data6[0]+48)!=(etat)) {
			printf("Ici \n");
			etat = data6[0] -48;
			if(etat) N=N1;
			else N=N2;
		}
		
		fd12 = open("/sys/class/gpio/gpio12/value",O_WRONLY);
		data[0] = 49;//1
		write(fd12,data,1);
		usleep(N);
		data[0] = 48;//0
		write(fd12,data,1);
		usleep(20000-N);
		close(fd12);
	}
}

int main(int argc, char **argv)
{	
	
	char dout[] = "out";
	char din[] = "in";
	char dstring[100];

	
	sprintf(dstring,"%d", 12);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,2);
	close(fd);

	
	fd12 = open("/sys/class/gpio/gpio12/direction",O_WRONLY);
	write(fd,dout,3);
	close(fd);
	
	sprintf(dstring,"%d",6);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,1);
	close(fd);

	
	fd6 = open("/sys/class/gpio/gpio6/direction",O_WRONLY);
	write(fd6,din,2);
	close(fd);
	
	
	printf("la\n");                                                                                            
	pthread_t t;
	printf("la\n");
	float testchiffre = 42.;
	//struct toto vartoto;
	//vartoto.n = 42;
	//sprintf(testtoto.name,"toto");
	pthread_create(&t,NULL,fait_tourner_servomoteur,&testchiffre);
	printf("la\n");
	

	while(1) {
		fd6 = open("/sys/class/gpio/gpio6/value",O_RDONLY);
		read(fd6,data6,1);
		close(fd6);
		usleep(10000);
	}
	printf("la\n");
	pthread_join(t,NULL);
	return 0 ;
}


                                   
