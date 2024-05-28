default: main

main: main.cpp calcul_angle_beta.cpp envois_consignes_moteurs.cpp calcul_consigne.cpp myWiringPi.cpp 
	g++ -o $@ $^ `pkg-config --cflags --libs opencv4` 
	
beta : calcul_angle_beta.cpp
	g++ -o $@ $< `pkg-config --cflags --libs opencv4`

calcul : calcul_consigne.cpp
	g++ -o $@ $< `pkg-config --cflags --libs opencv4`
	
mywiringpi : myWiringPi.cpp
	g++ -o $@ $< `pkg-config --cflags --libs opencv4`

