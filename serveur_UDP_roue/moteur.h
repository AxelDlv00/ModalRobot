#ifndef MOTEUR_H
#define MOTEUR_H

typedef struct moteur{
	
	int id;
	
	int DIR, EN, SA, SB;
	int SA_val1, SA_val0, SB_val1;
	
	float compteur_rad;
	
	float vit_consigne;
	float vit_consigne_prec;
	float vit_reelle;
	float erreur ;
	float vit_envoyee;
	int vit_envoyee_radkilo;
	float pwm;
	float Kp;
	float Kd;
	
	float erreur_precedente;
	float derivee_erreur;
	
	float cpt_t1, cpt_t2;
	long cpt;
	
	} Moteur;

#endif
