#include "calcul_angle.h"

#include <cmath> // Pour std::isnan
#include <cstdio> // Pour printf

/*
void calcul_angle(float dB[NB_MICROS]) {
    int maxIndex = -1;
    float maxValue = -INFINITY;
    
    // Cherche le micro ayant la plus grosse intensité mesurée
    for (int i = 0; i < NB_MICROS; i++) {
        if (!std::isnan(dB[i]) && dB[i] > maxValue) {
            maxIndex = i;
            maxValue = dB[i];
        }
    }
    
    // Vérifie si un indice valide a été trouvé
    if (maxIndex != -1) {
        // Calcule l'angle associé
        float angle_sortie = maxIndex * ANGLE_INCR;
        angle_consigne = angle_sortie;
    }
}
* */

void calcul_angle(float dB[NB_MICROS]) {
    float prod_scalaire_x = dB[0]-dB[3]+(dB[1]+dB[5]-dB[2]-dB[4])/2.0f;
    float prod_scalaire_y = (dB[1]-dB[5]+dB[2]-dB[4])/std::sqrt(2.0f);
    if(prod_scalaire_y != 0) angle_consigne = std::atan(prod_scalaire_y/prod_scalaire_x) / 4.0f;
    else angle_consigne = 0.0f;
}




void avancer(float dB[NB_MICROS]){
	int symIndex = NB_MICROS%2;
	float limite_dB = 2.0f;
	int test = 0;
	if (std::fabs(dB[0]-dB[symIndex])<limite_dB)
	{
		test = 1; //Avancer
	}
	else 
	{
		test = 0; //Stop
	}
	
	test_avancer = test;
}
/*
int main() {
    float dB[NB_MICROS] = {40, 25, 37, 3, 2, 28}; // Exemple de valeurs d'intensité sonore

    float maxOrientation = calcul_angle(dB);
    printf("Orientation %f\n", maxOrientation);    

    return 0;
}*/
