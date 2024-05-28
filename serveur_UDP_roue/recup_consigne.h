#ifndef CALCUL_CONSIGNE_H
#define CALCUL_CONSIGNE_H

#include "moteur.h"
#include "main.h"
#include "envois_consignes_moteurs.h"

void * recup_consigne(void * a);

void * calcul_consigne_translation(void *a);
void * calcul_consigne_rotation(void *a);

//void * calcul_consigne_rotation_plus_translation(void *a);

void * actualiser_consigne(void *a);
#endif






