#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

#include "calcul_angle_beta.h"
 
using namespace cv;
using namespace std;

//DEFINITION BORNES DE SELECTION COULEUR (pour du rouge)
#define low_H 81//84
#define high_H 104//99
#define low_S 123//61
#define high_S 220//135
#define low_V 110//197
#define high_V 252//255
#define PI 3.1415

#define L_MIN 100

extern float angle_consigne;
extern float deplacement_consigne;

float beta_err = 0.0f;
 
void * calcul_angle_beta(void * a)
{
	//IMAGE A RESIZER POUR LA RAPIDITE
	
	//VideoCapture cap(argc > 1 ? atoi(argv[1]) : 0);
	VideoCapture cap(0);
	//reduire la resolution à l'entrée 
	cap.set(CAP_PROP_FRAME_WIDTH,320);
	cap.set(CAP_PROP_FRAME_HEIGHT,240);	
	
	//CREATION MATRICES A AFFICHER EN RGB + HSV + MATRICE FINALE EN 0 et 1
	Mat frame_RGB, frame_HSV, frame_01, frame_RGB_cropped;
	
	//METTRE EN HSV SELECTIONNE
	while (true) {
		cap >> frame_RGB; //stocke l'image reçue à chaque instant dans frame_RGB
		
		if(frame_RGB.empty())
		{
		break;
		}

		int crop_height = 100; // Hauteur de la bande en bas
		int crop_y = frame_RGB.rows - crop_height; // Commence à partir du bas de l'image
		int crop_x = 0;
		int crop_width = frame_RGB.cols; // Largeur de l'image entière
		Rect roi(crop_x, crop_y, crop_width, crop_height);

		frame_RGB_cropped = frame_RGB(roi);
		
		// Converti de BGR à HSV
		cvtColor(frame_RGB_cropped, frame_HSV, COLOR_BGR2HSV); //COLOR_BGR2HSV = convertisseur RGB to HSV

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
		float moyenne_deplacement = 0.0f;
		
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

			line(frame_01_A, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
			
			float longueur_segment_au_carre = (pt2.x -pt1.x)*(pt2.x -pt1.x) + (pt2.y -pt1.y)*(pt2.y -pt1.y);
				
			if (longueur_segment_au_carre>L_MIN*L_MIN){
				nb_segments_consideres += 1 ;
				int dx, dy;
				dx = pt2.x-pt1.x;
				dy = pt2.y-pt1.y;
				float beta = std::atan((float) dx / (float) dy);
				moyenne_beta += beta;
				moyenne_deplacement += (pt2.x+pt1.x)/2 - frame_01.cols/2; // Ajouter la moyenne du deplacement de la droite 
			}
		}
		
		moyenne_beta /= nb_segments_consideres;
		moyenne_deplacement /= nb_segments_consideres;

		float ancien_beta = angle_consigne;
		float ancien_deplacement = deplacement_consigne;
		
		beta_err = moyenne_beta;
		if (std::isnan(beta_err)){
			angle_consigne = 0.;
		}
		else {
			angle_consigne = beta_err; 
		}

		if (std::isnan(moyenne_deplacement)){
			deplacement_consigne = 0.;
		}
		else {
			deplacement_consigne = moyenne_deplacement; 
		}
		
		float pas_temps = 30 * 1e-3;
		float f = 2;//0.4;
		float alpha = pas_temps / (1/(2*PI*f)+pas_temps);

		angle_consigne = alpha * angle_consigne + (1-alpha) * ancien_beta;
		deplacement_consigne = alpha * deplacement_consigne + (1-alpha) * ancien_deplacement;

		//printf("beta : %f\n", angle_consigne);
		//printf("deplacement : %f :\n ", deplacement_consigne);
		
		//printf("Angle beta moyen : %f \n", moyenne_beta);
		
		//imshow("Matrice 01", frame_01);
		//imshow("Lignes", frame_01_A);
		
		char key = (char) waitKey(30);
		
		if (key == 'q' || key == 27)
		{
			break;
		}
		
	}
	return a;
}

