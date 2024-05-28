#include "envois_consignes_moteurs.h"
#include "myWiringPi.h"

Moteur moteur1, moteur2, moteur3, moteuri;
Moteur* Liste_Moteur[3] = {&moteur1, &moteur2, &moteur3};

extern float V_Consigne_Globale;
extern int running;
extern float dt;
extern int etat;
extern int dejaactive; 
extern int estentraindappuyer;
extern int allume;


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void handler(int n){
	running=0;
	usleep(100);
	mydigitalWrite(moteur1.EN,0);
	mydigitalWrite(moteur2.EN,0);
	mydigitalWrite(moteur3.EN,0);
	printf("Fin de moteurs !\n");
	exit(0);

}


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
		usleep(100);
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
	return arg;
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
			//usleep(30);
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
	return arg;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

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
		//printf("Vitesse mesurée moteur %i : %f\n", m->id, m->vit_reelle);
		m->erreur = m->vit_consigne-m->vit_reelle;
		float f = 2;//0.4;
		float alpha = N_moy * dt / (1/(2*PI*f)+N_moy*dt);
		m-> erreur = alpha * m->erreur + (1-alpha) * m->erreur_precedente;
		m->derivee_erreur = (m->erreur-m->erreur_precedente)/(dt/1000000);
		m->erreur_precedente = m -> erreur;
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
	moteur1.Kd = KD1;
	
	moteur1.compteur_rad = 0;
	moteur1.vit_consigne = 0;
	moteur1.vit_reelle = 0.;
	moteur1.erreur = 0.;
	moteur1.vit_envoyee = 0.;
	moteur1.vit_envoyee_radkilo = 0;
	moteur1.erreur_precedente = 0.;
	moteur1.derivee_erreur = 0.;
	
	printf("init moteur 2\n");
	
	moteur2.id = 2;
	moteur2.DIR = DIR2;
	moteur2.EN = EN2;
	moteur2.SA = SA2;
	moteur2.SB = SB2;
	moteur2.Kp = KP2;
	moteur2.Kd = KD2;
	
	moteur2.compteur_rad = 0;
	moteur2.vit_consigne = 0;
	moteur2.vit_reelle = 0.;
	moteur2.erreur = 0.;
	moteur2.vit_envoyee = 0.;
	moteur2.vit_envoyee_radkilo = 0;
	moteur2.erreur_precedente = 0.;
	moteur2.derivee_erreur = 0.;
	
	printf("init moteur 3\n");

	moteur3.id = 3;
	moteur3.DIR = DIR3;
	moteur3.EN = EN3;
	moteur3.SA = SA3;
	moteur3.SB = SB3;
	moteur3.Kp = KP3;
	moteur3.Kd = KD3;
	
	moteur3.compteur_rad = 0;
	moteur3.vit_consigne = 0;
	moteur3.vit_reelle = 0.;
	moteur3.erreur = 0.;
	moteur3.vit_envoyee = 0.;
	moteur3.vit_envoyee_radkilo = 0;
	moteur3.erreur_precedente = 0.;
	moteur3.derivee_erreur = 0.;
	
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
		//printf("Vitesse consigne moteur %i : %f\n", m->id, m->vit_consigne);
		//printf("Pwm moteur %i : %f\n", m->id, m->pwm);
		allume = HIGH;
		if (allume == HIGH){
			m->vit_envoyee = m->vit_consigne + (m->Kp)*(m->erreur); //+ (m->Kd)*(m->derivee_erreur);
			//printf("Erreur moteur %i : %f, derivee_erreur : %f\n", m->id, m->erreur,(m->derivee_erreur));
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
		usleep(1000);
	}
	return moteur;
}
