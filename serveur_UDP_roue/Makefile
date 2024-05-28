default: main

main: main.cpp recup_consigne.cpp envois_consignes_moteurs.cpp serveur_udp.cpp myWiringPi.cpp 
	g++ -o $@ $^ 
	
beta : calcul_angle_beta.cpp
	g++ -o $@ $<

calcul : calcul_consigne.cpp
	g++ -o $@ $<
	
mywiringpi : myWiringPi.cpp
	g++ -o $@ $<
	
ugp: serveur_udp.cpp 
	g++ -o $@ $<
	
recup : recup_consigne.cpp
	g++ -o $@ $< 


