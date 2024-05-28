#include "myWiringPi.h"

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
