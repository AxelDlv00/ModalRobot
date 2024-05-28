default: test 

autres: 
	gcc -o myprogram1 test2.c
	gcc -o myprogram2 readwrite.c
	gcc -o myprogram3 moteur.c
	gcc -o myprogram4 moteur_test_Solange.c
	gcc -o myprogram5 servowiring.c -lwiringPi
	gcc -o myprogram6 moteur_incr.c -lwiringPi
	gcc -o myprogram7 capteur_incr.c -lwiringPi
	gcc -o myprogram8 asservissement_vitesse.c -lwiringPi
	gcc -o myprogram9 test_1_moteur.c -lwiringPi
	gcc -o myprogram10 asservissement_plusieurs_moteurs.c -lwiringPi
	gcc -o myprogram11 controle_vitesse_3_moteurs.c -lwiringPi
	g++ -o myprogram12 testcamera.cpp `pkg-config --cflags --libs opencv4`
	g++ -o test_line test_line.cpp `pkg-config --cflags --libs opencv4`
	g++ -o testdetectiondelignes testdetectiondelignes.cpp `pkg-config --cflags --libs opencv4`
	
hough: combiner_camera_hough.cpp
	g++ -o $@ $< `pkg-config --cflags --libs opencv4`
	
findcontourcamera: findcontourcamera.cpp
	g++ -o $@ $<  `pkg-config --cflags --libs opencv4`
	
testline: test_line.cpp
	g++ -o $@ $<  `pkg-config --cflags --libs opencv4`
	
testcamera : testcamera.cpp
	g++ -o myprogram12 testcamera.cpp `pkg-config --cflags --libs opencv4`

asservcamera : asserv_camera.cpp
	g++ -o `pkg-config --cflags --libs opencv4`
	
test :  controle_vitesse_3_moteurs.c
	gcc -o $@ $<  -lwiringPi
