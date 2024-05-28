#ifndef CALCUL_ANGLE
#define CALCUL_ANGLE
#include <stdio.h>
#include <cmath>
#include "main.h"

#define NB_MICROS 6
#define PI 3.1415926535089793
#define ANGLE_INCR 2*PI/NB_MICROS

extern float angle_consigne;
extern int test_avancer;

void calcul_angle(float dB[NB_MICROS]);
void avancer(float dB[NB_MICROS]);

#endif
