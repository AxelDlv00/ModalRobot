#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Étape 1: Lire l'imagess
    Mat img = imread("tacheblanche.jpg");
    if(img.empty()) {
        cout << "Erreur lors du chargement de l'image" << endl;
        return -1;
    }
    resize(img, img, Size(), 0.01, 0.01, INTER_LINEAR);

    // Étape 2: Convertir en niveaux de gris
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    // Étape 3: Appliquer un seuillage
    Mat thresh;
    threshold(gray, thresh, 127, 255, THRESH_BINARY);

    // Étape 4: Trouver les contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(thresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // Étape 5: Dessiner les contours
    Mat imgContours = Mat::zeros(img.size(), img.type());
    for(int i = 0; i< contours.size(); i++) {
        drawContours(imgContours, contours, i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0);
    }

    // Afficher l'image originale et l'image avec contours
    imshow("Image Originale", img);
    imshow("Image avec Contours", imgContours);
    waitKey(0);

    return 0;
}
