#ifndef CALCUL_ANGLE_BETA_H
#define CALCUL_ANGLE_BETA_H

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include "main.h"

using namespace cv;
using namespace std;

// Définition des bornes de sélection de couleur (pour le rouge)
#define low_H 81//84
#define high_H 104//99
#define low_S 123//61
#define high_S 220//135
#define low_V 110//197
#define high_V 252//255

#define L_MIN 100

extern float beta_err;

// Fonction pour calculer la moyenne de l'angle beta
void * calcul_angle_beta(void * a);

#endif
