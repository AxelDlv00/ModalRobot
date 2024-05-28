// Pour la camera
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

// Pour le contrôle des moteurs
#include <stdio.h> //fct de base
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h> //pour thread
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
 
using namespace cv;
using namespace std;

//DEFINITION BORNES DE SELECTION COULEUR (pour du rouge)
#define low_H 81//84
#define high_H 104//99
#define low_S 123//61
#define high_S 220//135
#define low_V 110//197
#define high_V 252//255

#define L_MIN 100

float dot(Point a, Point b){
	return a.x*b.x+a.y*b.y;}
 
int main(int argc, char** argv)
{
	VideoCapture cap(argc > 1 ? atoi(argv[1]) : 0);
	
	//CREATION MATRICES A AFFICHER EN RGB + HSV + MATRICE FINALE EN 0 et 1
	Mat frame_RGB, frame_HSV, frame_01;
	
	//METTRE EN HSV SELECTIONNE
	while (true) {
		cap >> frame_RGB; //stocke l'image reçue à chaque instant dans frame_RGB
		
		if(frame_RGB.empty())
		{
		break;
		}
		
		// Converti de BGR à HSV
		cvtColor(frame_RGB, frame_HSV, COLOR_BGR2HSV); //COLOR_BGR2HSV = convertisseur RGB to HSV

		///////////// DECALAGE COULEUR /////////////// = pour éviter de devoir joindre deux intervalles
		for(int i = 0; i < frame_HSV.rows; i++) {
			for(int j = 0; j < frame_HSV.cols; j++) {
				unsigned char& H = frame_HSV.at<Vec3b>(i, j)[0];
				H = (H + 90) % 180; // Utilisez 90 pour π/2 ou toute autre valeur pour une translation différente
			}
		}

		// Creation matrice 01 en excluant (noir) les valeurs hors de l'intervalle HSV défini précédemment
		inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_01); //MATRICE 01 AVEC VALEURS DANS INTERVALLE
		
		
		//////////////////////////////////////////////////// TRANSFORMEE DE HOUGH //////////////////////////////////////////////////////////
		
		Mat frame_01_A, frame_01_B; //MATRICES DE SORTIES
		
		cvtColor(frame_01, frame_01_A, COLOR_GRAY2BGR);
		
		frame_01_B = frame_01_A.clone();
		
		float moyenne_rho = 0.0f;
		float moyenne_theta = 0.0f;
		float moyenne_beta = 0.0f;
		
		/*//Calcul moyenne avec Standard Hough Line Transform = lignes complète sans exclusion des segments trop courts
		vector<Vec2f> lines; // will hold the results of the detection
		HoughLines(frame_01, lines, 1, CV_PI/180, 150, 0, 0 ); // runs the actual detection
		
		for( size_t i = 0; i < lines.size(); i++ )
		{
			float rho = lines[i][0], theta = lines[i][1];
			moyenne_rho += rho;
			moyenne_theta += theta;
		}
		
		moyenne_rho /= lines.size();
		moyenne_theta /= lines.size();
		
		printf("Rho moyen : %f ; Theta moyen : %f\n", moyenne_rho, moyenne_theta);
		*/
		
		/////////// 	Calcul moyenne avec Probabilistic Line Transform (segment finis) 	///////////
		
		vector<Vec4i> linesP; // will hold the results of the detection
		HoughLinesP(frame_01, linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection
		
		int nb_segments_consideres = 0;
		
		for( size_t i = 0; i < linesP.size(); i++ )
		{
			Vec4i l = linesP[i];
			Point pt1, pt2;
			
			pt1.x = l[0];
			pt1.y = l[1];
			pt2.x = l[2];
			pt2.y = l[3];
			
			float longueur_segment_au_carre = (pt2.x -pt1.x)*(pt2.x -pt1.x) + (pt2.y -pt1.y)*(pt2.y -pt1.y);
			
			if (longueur_segment_au_carre > l_max_2)
				l_max_2 = longueur_segment_au_carre;
			if (longueur_segment_au_carre < l_min_2)
				l_min_2 = longueur_segment_au_carre;
				
			if (longueur_segment_au_carre>L_MIN*L_MIN){
				nb_segments_consideres += 1 ;
				int dx, dy;
				dx = pt2.x-pt1.x;
				dy = pt2.y-pt1.y;
				float beta = std::atan((float) dx / (float) dy);
				moyenne_beta += beta;
			}
		}
		
		moyenne_beta /= nb_segments_consideres;
		
		printf("Angle beta moyen : %f \n", moyenne_beta);
		
		imshow("Matrice 01", frame_01);
		
		char key = (char) waitKey(30);
		
		if (key == 'q' || key == 27)
		{
			break;
		}
		
	}
	return 0;
}

