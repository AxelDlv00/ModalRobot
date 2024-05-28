//#include <opencv2/core.hpp>
//#include <opencv2/imgproc/imgproc_c.h>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace cv;
const int max_value_H = 360/2;
const int max_value = 255;
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";
int low_H = 0, low_S = 0, low_V = 0;
int high_H = max_value_H, high_S = max_value, high_V = max_value;

//DEFINITION BORNES DE SELECTION COULEUR
static void on_low_H_thresh_trackbar(int, void *)
{
 low_H = min(high_H-1, low_H);
 setTrackbarPos("Low H", window_detection_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
 high_H = max(high_H, low_H+1);
 setTrackbarPos("High H", window_detection_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
 low_S = min(high_S-1, low_S);
 setTrackbarPos("Low S", window_detection_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
 high_S = max(high_S, low_S+1);
 setTrackbarPos("High S", window_detection_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
 low_V = min(high_V-1, low_V);
 setTrackbarPos("Low V", window_detection_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
 high_V = max(high_V, low_V+1);
 setTrackbarPos("High V", window_detection_name, high_V);
}

// FONCTION MAIN
int main(int argc, char* argv[])
{
 VideoCapture cap(argc > 1 ? atoi(argv[1]) : 0);
 namedWindow(window_capture_name);
 namedWindow(window_detection_name);
 // Trackbars to set thresholds for HSV values // CREATION BARRES DE SELECTION
 createTrackbar("Low H", window_detection_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
 createTrackbar("High H", window_detection_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
 createTrackbar("Low S", window_detection_name, &low_S, max_value, on_low_S_thresh_trackbar);
 createTrackbar("High S", window_detection_name, &high_S, max_value, on_high_S_thresh_trackbar);
 createTrackbar("Low V", window_detection_name, &low_V, max_value, on_low_V_thresh_trackbar);
 createTrackbar("High V", window_detection_name, &high_V, max_value, on_high_V_thresh_trackbar);
 //CREATION MATRICES A AFFICHER EN RGB ET HSV + MATRICE FINALE EN 0 et 1
 Mat frame, frame_HSV, frame_threshold;
 while (true) {
 cap >> frame;
 if(frame.empty())
 {
 break;
 }
 // Convert from BGR to HSV colorspace
 cvtColor(frame, frame_HSV, COLOR_BGR2HSV); //COLOR_BGR2HSV = convertisseur RGB to HSV

///////////// TEST ///////////////

for(int i = 0; i < frame_HSV.rows; i++) {
    for(int j = 0; j < frame_HSV.cols; j++) {
        unsigned char& H = frame_HSV.at<Vec3b>(i, j)[0];
        H = (H + 90) % 180; // Utilisez 90 pour π/2 ou toute autre valeur pour une translation différente
    }
}

// Convertissez l'image modifiée de retour en BGR = après conversion
cvtColor(frame_HSV, frame, COLOR_HSV2BGR); //COLOR_BGR2HSV = convertisseur RGB to HSV

////////// FIN DU TEST /////////////

 // Detect the object based on HSV Range Values
 inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), frame_threshold); //MATRICE 01 AVEC VALEURS DANS INTERVALLE
 // Show the frames
 imshow(window_capture_name, frame);
 imshow(window_detection_name, frame_threshold);
 char key = (char) waitKey(30);
 if (key == 'q' || key == 27)
 {
 break;
 }
 }
 return 0;
}
