#include <stdio.h>
#include <stdlib.h>
#include <math.h> // se pune -lm ca ultim argument pt compilarea cu gcc
#include <stdbool.h>

#define PI 3.1415

float calculeazaUnghi(float xOld, float yOld, float xNew, float yNew, float x, float y, float deviere){
	// xOld, yOld - coordonatele de la pasul anterior al robotului nostru
	// xNew, yNew - coordonatele de la pasul curent al robotului nostru
	// x, y - coordonatele de la pasul curent al celuilalt robot

	float unghi1, unghi2, unghi;
	bool miscareVerticala, miscareOrizontala;


	// robotul nostru

	if (yOld - deviere <= yNew && yNew <= yOld + deviere){
		miscareVerticala = true;
	} else {
		miscareVerticala = false;
	}

	if (xOld - deviere <= xNew && xNew <= xOld + deviere){
		miscareOrizontala = true;
	} else {
		miscareOrizontala = false;
	}


	if(miscareVerticala == true && miscareOrizontala == true){
		// sta pe loc sau are o deviere foarte mica (suntem impinsi in spate sau impingem adversarul)
		
		// de memorat pozitia initiala
		// dupa un numar de secunde vom verifica daca impingem, suntem impinsi sau stam pe loc si vom incerca sa il rotim (daca nu il impingem)
		
		// DE IMBUNATATIT

		return 0.0; // nu ne rotim, ci continuam sa mergem in fata/impingem
	} else if(miscareVerticala == true){
		// miscare pe verticala
		if (xOld < xNew){
			unghi1 = 270.0;
		} else{
			unghi1 = 90.0;
		}
	} else if (miscareOrizontala == true){
		// miscare pe orizontala
		if (yOld < yNew){
			unghi1 = 0.0;
		} else{
			unghi1 = 180.0;
		}
	} else{
		// miscare oblica
		unghi1 = atan2(yNew - yOld, xOld - xNew) / PI * 180.0;
	}


	// robotul adversar

	// DE VERIFICAT DACA SUNTEM LANGA ROBOTUL ADVERSARULUI

	// DE INLOCUIT PARAMETRUL `deviere` cu o alta valoare. `deviere` de la traiectoria noastra nu este acelasi lucru cu `deviere` de la traiectoria spre robotul adversarului
	

	if (yNew - deviere <= y && y <= yNew + deviere){
		// miscare pe verticala
		if (xNew < x){
			unghi2 = 270.0;
		} else{
			unghi2 = 90.0;
		}
	} else if (xNew - deviere <= x && x <= xNew + deviere){
		// miscare pe orizontala
		if (yNew < y){
			unghi2 = 0.0;
		} else{
			unghi2 = 180.0;
		}
	} else{
		// miscare oblica
		unghi2 = atan2(y - yNew, xNew - x) / PI * 180.0;
	}


	// DE STERS DUPA TESTARE !!!
	printf("unghi_1 = %f ; unghi_2 = %f\n", unghi1, unghi2);

	unghi =  unghi2 - unghi1;

	if (unghi < -180.0){
		return unghi + 360.0;
	}
	else if (unghi > 180.0){
		return unghi - 360.0;
	}
	return unghi;
}

void move(char dir, float unghi){
	if (dir == 'l'){
		printf("Se roteste la stanga cu %f grade\n", unghi);
	}
	else if (dir == 'r'){
		printf("Se roteste la dreapta cu %f grade\n", unghi);
	}

	// apoi se misca in fata x ms
	printf("Se deplaseaza in fata\n\n");
}

int main(void){ // lagoritm

	float xOld, yOld, xNew, yNew, x, y, eroare, deviere;

	scanf("%f%f%f%f%f%f%f%f", &xOld, &yOld, &xNew, &yNew, &x, &y, &eroare, &deviere);

	float unghi = calculeazaUnghi(xOld, yOld, xNew, yNew, x, y, deviere);

	printf("\n%f\n\n", unghi);

	// ca sa nu facem atat de multe miscari de rotatie (pentru ca nu putem obtine miscari
	// fine de rotatie) putem sa verificam daca eroarea traiectoriei noastre  este mai 
	// mica decat o EROARE MAXIMA

	if (unghi < -eroare){
		move('l', unghi); // left
	}
	else if (unghi > eroare){
		move('r', unghi); // right
	}
	else{
		move('x', unghi);
	}

	return 0;
}
