#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char data6[2]; // Appuyé ou pas
	char data5[2]; // Allumé ou pas
	int dejaactive = 0; 
	int estentraindappuyer = 0;
	
	int fd5, fd6, fd;
	
	char dout[] = "out";
	char din[] = "in";
	char dstring[100];
	
	sprintf(dstring,"%d",5);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,1);
	close(fd);
	
	sprintf(dstring,"%d",6);
	fd = open("/sys/class/gpio/export",O_WRONLY);
	write(fd,dstring,1);
	close(fd);
	
	fd = open("/sys/class/gpio/gpio5/direction",O_WRONLY);
	write(fd,dout,3);
	close(fd);
	
	fd = open("/sys/class/gpio/gpio6/direction",O_WRONLY);
	write(fd,din,2);
	close(fd);
	
	fd5 = open("/sys/class/gpio/gpio5/value",O_WRONLY); //Initialisation à éteint
	data5[0] = 48;
	read(fd5,data5,1);
	close(fd5);
	
	int allume = 0;
	
	while (1) {
		fd6 = open("/sys/class/gpio/gpio6/value",O_RDONLY); //Lit l'appui
		read(fd6,data6,1);
		close(fd6);

		fd5 = open("/sys/class/gpio/gpio5/value",O_RDWR); //Lit lumière
		read(fd5,data5,1);
		
		estentraindappuyer = (1-data6[0]+48); // 1 si appuyé 0 sinon
		
		if (estentraindappuyer&&(!dejaactive)) { //appuyé
			if (allume){
				data5[0]= 48; //Eteindre
				allume = 0;
			}
			else {
				data5[0]= 49;//Allumer
				allume = 1;
			}
			printf("data%i %i \n",6,data6[0]);
			printf("data%i %i \n",5,data5[0]);
			
			dejaactive = 0;
		}
		
		
		write(fd5,data5,1);
		close(fd5);
	}

	return 0 ;
}
                                   
