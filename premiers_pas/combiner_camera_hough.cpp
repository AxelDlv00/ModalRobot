#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
 
using namespace cv;
using namespace std;

//DEFINITION BORNES DE SELECTION COULEUR (pour du rouge)
#define low_H 81//84
#define high_H 104//99
#define low_S 123//61
#define high_S 220//135
#define low_V 110//197
#define high_V 252//255
 
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
		// Standard Hough Line Transform
		vector<Vec2f> lines; // will hold the results of the detection
		HoughLines(frame_01, lines, 1, CV_PI/180, 150, 0, 0 ); // runs the actual detection
		// Draw the lines
		for( size_t i = 0; i < lines.size(); i++ )
		{
			float rho = lines[i][0], theta = lines[i][1];
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			line(frame_01_A, pt1, pt2, Scalar(0,0,255), 3, LINE_AA);
		}

		// Probabilistic Line Transform
		vector<Vec4i> linesP; // will hold the results of the detection
		HoughLinesP(frame_01, linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection
		
		// Draw the lines
		for( size_t i = 0; i < linesP.size(); i++ )
		{
			Vec4i l = linesP[i];
			line( frame_01_B, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
		}

		
		// Show the frames
		imshow("Image camera", frame_RGB);
		imshow("Matrice 01", frame_01);
		imshow("Standard Hough Line Transform - lignes", frame_01_A);
		imshow("Probabilistic Line Transform - segment", frame_01_B);		
		
		char key = (char) waitKey(30);
		
		if (key == 'q' || key == 27)
		{
			break;
		}
		
	}
	return 0;
}
