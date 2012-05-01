/* Pliki nagłówkowe */

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

#include <gl/gl.h> 
#include <gl/glut.h>

//LINUX
#include <GL/gl.h>
#include <GL/glut.h>

//WINDOWS
//#include <gl/gl.h>
//#include "glut.h"
//#include <windows.h>


/* Dyrektywy dla preprocesora */

#define ELEMENTY 	200
#define WSPOLRZEDNE 3


/* Zmienne globalne */

// Wymiary okna
int screen_width  = 640;
int screen_height = 480;

// Zmienne do kombinacji kamera + mysz
int pozycjaMyszyX = 0;
int pozycjaMyszyY = 0;

double kameraX        = 0;
double kameraZ        = 0;
double kameraPredkosc = 0;

double kameraKat            = 0;
double kameraPredkoscObrotu = 0;

// Zmienne od obrotu
GLfloat dx = 1;
GLfloat dy = 1;
GLfloat dz = 1;

int jakaSkala = 6;		// Patrz enum

// Maksymalne i minimalne wartości poszczególnych współrzędnych
double maxX = 0;
double maxY = 0;
double maxZ = 0;

double minX = 0;
double minY = 0;
double minZ = 0;

// Zmienne do sterowania reflektorem
GLfloat spotPosX = 0;
GLfloat spotPosY = 0;
GLfloat spotPosZ = 2;

// Nazwa pliku
const char nazwaPliku[] = "tatry_dane.txt";

// Tablica do przechowywania danych
double tablicaDane[ELEMENTY][ELEMENTY][WSPOLRZEDNE];


/* Nagłówki funkcji */

void resetKamery(void);

void mysz       (int, int, int, int);		// Obsługa myszy
void mysz_ruch  (int, int);					// Obsługa ruchu myszy

void klawisz    (GLubyte, int, int);		// Obsługa klawiszy

void rozmiar    (int, int);
void rysuj      (void);

void wczytajDane(void);

void wyznaczMax		  (void);
void wyznaczMin		  (void);
void normalizujTablice(void);

void normalizacjaWektora(float wektor[3]);
void obliczNormalne     (float v[3][3], float out[3]);

void menuSwiatlo(int);
void menuSkala  (int);


/* Enum do menu kontekstowego */

enum
{
	WLACZ_REFL,
	WYLACZ_REFL,
	WLACZ_KIER,
	WYLACZ_KIER,
	
	SKALA_SZARA,
	SKALA_KOLOR,
	SKALA_SIATKA,
	
	EXIT
};


/* MAIN */

int main(int argc, char** argv)
{
	glutInit(&argc, argv);    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Tatry");    

	glutDisplayFunc(rysuj);			// def. funkcji rysującej
    glutIdleFunc(rysuj);			// def. funkcji rysującej w czasie wolnym procesoora (w efekcie: ciągle wykonywanej)
    glutReshapeFunc(rozmiar);		// def. obsługi zdarzenia resize (GLUT)
	
	glutKeyboardFunc(klawisz);		// def. obsługi klawiatury
    glutMouseFunc(mysz); 			// def. obsługi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch); 		// def. obsługi zdarzenia ruchu myszy (GLUT)
	
	resetKamery();
	
	
	/* --- Przygotowanie danych --- */
	
	wczytajDane();
	wyznaczMax();
	wyznaczMin();
	normalizujTablice();
	
	
	/* --- Menu kontekstowe --- */
	
	int Swiatlo = glutCreateMenu(menuSwiatlo);
	glutAddMenuEntry("Wlacz reflektor", WLACZ_REFL);
	glutAddMenuEntry("Wylacz reflektor", WYLACZ_REFL);
	glutAddMenuEntry("Wlacz swiatlo kierunkowe", WLACZ_KIER);
	glutAddMenuEntry("Wylacz swiatlo kierunkowe", WYLACZ_KIER);
	
	int Skala = glutCreateMenu(menuSkala);
	glutAddMenuEntry("Skala szarosci", SKALA_SZARA);
	glutAddMenuEntry("Skala kolorow", SKALA_KOLOR);
	glutAddMenuEntry("Bez skali", SKALA_SIATKA);
	
	glutCreateMenu(menuSwiatlo);
    glutAddSubMenu("Swiatlo", Swiatlo);
	glutAddSubMenu("Skala", Skala);
	glutAddMenuEntry("Wyjscie", EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	/* ---------------------- */
	
	glEnable(GL_DEPTH_TEST); 
	
	glutMainLoop();	

    return 0; 
}


/* Definicje funkcji */

void resetKamery()
{
	kameraX              = 0;
	kameraZ              = 0;
	kameraPredkosc       = 0;
	kameraKat            = 0;
	kameraPredkoscObrotu = 0;
}

void mysz(int button, int state, int x, int y)
{
	switch(state)
	{
		case GLUT_UP:
			kameraPredkosc       = 0;
			kameraPredkoscObrotu = 0;
			break;
		case GLUT_DOWN:
			pozycjaMyszyX = x;
			pozycjaMyszyY = y;
			break;
	}
}

void klawisz(GLubyte k, int x, int y)
{
	switch(k) 
	{
		case 27:
			exit(1);
			break;
		case ' ':    
			glutFullScreen();
			break;
		// Obroty
		case 'o':
			dx += 1;
			break;
		case 'k':
			dy += 1;
			break;
		case 'n':
			dz += 1;
			break;
		case 'p':
			dx -= 1;
			break;
		case 'l':
			dy -= 1;
			break;
		case 'm':
			dz -= 1;
			break;
		// Sterowanie reflektorem
		case 'w':    
			spotPosX += 0.01;
			break;
		case 's':    
			spotPosX -= 0.01;
			break;
		case 'a':    
			spotPosY += 0.01;
			break;
		case 'd':    
			spotPosY -= 0.01;
			break;
		case 'q':    
			spotPosZ += 0.01;
			break;
		case 'e':    
			spotPosZ -= 0.01;
			break;
   }  
}

void mysz_ruch(int x, int y)
{
	kameraPredkoscObrotu = -(pozycjaMyszyX - x) * 0.00001;
	kameraPredkosc       = (pozycjaMyszyY - y) * 0.001;
}

// --- ROZMIAR ---
void rozmiar(int width, int height)
{
    screen_width  = width; 
    screen_height = height; 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glViewport(0, 0, screen_width, screen_height); 
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); 
    gluPerspective(60.0, (GLfloat)screen_width/(GLfloat)screen_height, 1.0, 1000.0);
    glutPostRedisplay(); 			// Przerysowanie sceny
}

// --- RYSOWANIE ---
void rysuj(void)
{
	int k1 = 0;
	int k2 = 0;
	
	float wektorNormalny[3];
	float v[3][3] = { {1.0, 0.0, 1.0},
					  {0.0, 1.0, 0.0},
		              {-1.0, 0.0, 1.0} };
	
	
	/* --- Definicje świateł --- */
	
	// Reflektor
	GLfloat ambientLightR[4]  = {0.3, 0.3, 0.3, 1.0};
	GLfloat diffuseLightR[4]  = {0.5, 0.5, 0.5, 1.0};
	GLfloat specularLightR[4] = {0.8, 0.8, 0.8, 1.0};
	GLfloat	lightPosR[4]      = {spotPosX, spotPosY, spotPosZ, 1.0};
	GLfloat spotDirR[4]       = {0.0, 0.0, -3.0, 1.0};
	
	// Światło kierunkowe
	GLfloat ambientLightK[4]  = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuseLightK[4]  = {0.4, 0.4, 0.4, 1.0};
	GLfloat specularLightK[4] = {0.5, 0.5, 0.5, 1.0};
	GLfloat	lightPosK[4]      = {0.0, 0.0, 3.0, 1.0};
	
	/* ---------------------- */
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();	
	
	// Tło
	glClearColor(0.1, 0.1, 0.1, 0.0);		
	
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//glEnable(GL_COLOR_MATERIAL); 
	
	
	/* --- Myszka i kamera --- */
	
	// Obsługa myszką sceny
    kameraKat += kameraPredkoscObrotu;		
	kameraX   += kameraPredkosc * sin(kameraKat);
    kameraZ   -= kameraPredkosc * cos(kameraKat);
	
	// Kamera
	gluLookAt(kameraX, 0, kameraZ, kameraX + 100 * sin(kameraKat), 0, kameraZ - 100*cos(kameraKat), 0, 1, 0); 
	
	
	/* --- Obsługa świateł --- */
	
// Wyłączone, bo coś jest nie tak!
	//glEnable(GL_LIGHTING);
	
	// Reflektor
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLightR);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLightR);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLightR);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosR);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 3.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 6.0);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirR);
	
	// Światło kierunkowe
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLightK);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLightK);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLightK);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosK);
	
	/* ---------------------- */
	
	
	// Ustawienia początkowe sceny
	glTranslatef(-0.5, 0.3, -2.0);
	glRotatef(35, 1.0, 0.0, 0.0);
	
	// Funkcje potrzebne do obrotów
	glRotatef(dx, 1.0, 0.0, 0.0);
    glRotatef(dy, 0.0, 1.0, 0.0);
    glRotatef(dz, 0.0, 0.0, 1.0);
	
	
	obliczNormalne(v, wektorNormalny);
	
	
	/* --- Rysowanie i "kolorowanie" --- */
	
	if(jakaSkala == SKALA_SZARA)
	{
		for(k2 = 0; k2 < ELEMENTY; k2++)
			for(k1 = 0; k1 < ELEMENTY; k1++)
			{
				glBegin(GL_TRIANGLES);		// dzielimy wysokość przez 8.0, żeby górki były mniejsze i lepiej wyglądały ;-)
				{
					glNormal3fv(wektorNormalny);
				
					glColor3f(tablicaDane[k1][k2][1],tablicaDane[k1][k2][1],tablicaDane[k1][k2][1]);                    
					glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);

					glColor3f(tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1]);
					glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);

					glColor3f(tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1]);
					glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				}  
				glEnd();
			}
	}
	else if(jakaSkala == SKALA_KOLOR)
	{
		for(k2 = 0; k2 < ELEMENTY; k2++)
			for(k1 = 0; k1 < ELEMENTY; k1++)
			{
				glBegin(GL_TRIANGLES);		// dzielimy wysokość przez 8.0, żeby górki były mniejsze i lepiej wyglądały ;-)
				{
					glNormal3fv(wektorNormalny);
				
					glColor3f(tablicaDane[k1][k2][0],tablicaDane[k1][k2][2],tablicaDane[k1][k2][1]);                     
					glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);

					glColor3f(tablicaDane[k1+1][k2][0],tablicaDane[k1+1][k2][2],tablicaDane[k1+1][k2][1]);
					glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);

					glColor3f(tablicaDane[k1+1][k2+1][0],tablicaDane[k1+1][k2+1][2],tablicaDane[k1+1][k2+1][1]);
					glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				}  
				glEnd();
			}
	}
	else if(jakaSkala == SKALA_SIATKA)
	{
		glColor3f(1.0, 0.0, 0.0);
		
		for(k2 = 0; k2 < ELEMENTY; k2++)
			for(k1 = 0; k1 < ELEMENTY; k1++)
			{
				glBegin(GL_TRIANGLES);		// dzielimy wysokość przez 8.0, żeby górki były mniejsze i lepiej wyglądały ;-)
				{
					glNormal3fv(wektorNormalny);
				
					glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);
					glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);
					glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				}  
				glEnd();
			}
	}
	
	/* ---------------------- */
  
    //glDisable(GL_COLOR_MATERIAL);
	
    glFlush(); 				// Rysowanie w buforze
    glutSwapBuffers(); 		// Wysłanie na ekran
}

void wczytajDane(void)
{
	int k1 = 0;
	int k2 = 0;
	
	FILE* f;
	
	if((f = fopen(nazwaPliku, "r")) == NULL)
	{
		printf("\nDEBUG: File error\n");
		exit(-1);
	}
	
	for(k1 = 0; k1 < ELEMENTY; k1++)
		for(k2 = 0; k2 < ELEMENTY; k2++)
		{
			fscanf(f, "%lf%lf%lf", &tablicaDane[k1][k2][0], &tablicaDane[k1][k2][1], &tablicaDane[k1][k2][2]);
		}
	
	fclose(f);
}

void wyznaczMax(void)
{
	int k1 = 0;
	int k2 = 0;
	
	maxX = tablicaDane[0][0][0];
	maxY = tablicaDane[0][0][1];
	maxZ = tablicaDane[0][0][2];
	
	for(k1 = 1; k1 < ELEMENTY; k1++)
		for(k2 = 1; k2 < ELEMENTY; k2++)
		{
			if(tablicaDane[k1][k2][0] > maxX)
				maxX = tablicaDane[k1][k2][0];
			
			if(tablicaDane[k1][k2][1] > maxY)
				maxY = tablicaDane[k1][k2][1];
				
			if(tablicaDane[k1][k2][2] > maxZ)
				maxZ = tablicaDane[k1][k2][2];
		}
}

void wyznaczMin(void)
{
	int k1 = 0;
	int k2 = 0;
	
	minX = tablicaDane[0][0][0];
	minY = tablicaDane[0][0][1];
	minZ = tablicaDane[0][0][2];
	
	for(k1 = 1; k1 < ELEMENTY; k1++)
		for(k2 = 1; k2 < ELEMENTY; k2++)
		{
			if(tablicaDane[k1][k2][0] < minX)
				minX = tablicaDane[k1][k2][0];
			
			if(tablicaDane[k1][k2][1] < minY)
				minY = tablicaDane[k1][k2][1];
				
			if(tablicaDane[k1][k2][2] < minZ)
				minZ = tablicaDane[k1][k2][2];
		}
}

void normalizujTablice(void)
{
	int k1 = 0;
	int k2 = 0;
	
	for(k1 = 0; k1 < ELEMENTY; k1++)
		for(k2 = 0; k2 < ELEMENTY; k2++)		// Kombinacja ta spowodowana jest tym, że wartości współrzędnych geograficznych mieszczą się w przedziale ~ [0;0.3]
		{
			tablicaDane[k1][k2][0] -= minX;
			tablicaDane[k1][k2][0] /= (maxX - minX);
			
			tablicaDane[k1][k2][1] -= minY;
			tablicaDane[k1][k2][1] /= (maxY - minY);
			
			tablicaDane[k1][k2][2] -= minZ;
			tablicaDane[k1][k2][2] /= (maxZ - minZ);
		}
}

/* --- Wektory normalne --- */

void normalizacjaWektora(float wektor[3]) 
{ 
	float dlugosc = (float)sqrt((wektor[0]*wektor[0]) + (wektor[1]*wektor[1]) + (wektor[2]*wektor[2])); // Oblicz długość wektora 
	
	if(dlugosc == 0.0f) 	// Zabezpieczenie przed podziałem przez 0 
		dlugosc = 1.0f; 
	
	// Podziel każdą ze współrzędnych przez długość wektora 
	wektor[0] /= dlugosc; 
	wektor[1] /= dlugosc; 
	wektor[2] /= dlugosc;
}

// Punkty p1, p2 i p3 zdefiniowane w odwrotnym do wskazówek zegara porządku 
void obliczNormalne(float v[3][3], float out[3]) 
{ 
	float v1[3]; 
	float v2[3]; 
	
	static const int x = 0; 
	static const int y = 1; 
	static const int z = 2; 
	
	// Oblicz 2 wektory na podstawie trzech punktów 
	v1[x] = v[0][x] - v[1][x]; 
	v1[y] = v[0][y] - v[1][y]; 
	v1[z] = v[0][z] - v[1][z]; 
	v2[x] = v[1][x] - v[2][x]; 
	v2[y] = v[1][y] - v[2][y]; 
	v2[z] = v[1][z] - v[2][z]; 
	
	// Oblicz współrzędne wektora normalnego na podstawie iloczynu wektorowego 
	out[x] = v1[y]*v2[z] - v1[z]*v2[y]; 
	out[y] = v1[z]*v2[x] - v1[x]*v2[z]; 
	out[z] = v1[x]*v2[y] - v1[y]*v2[x]; 
	
	// Normalizuj wektor 
	normalizacjaWektora(out); 
}

/* ---------------------- */

void menuSwiatlo(int wartosc)
{
	switch(wartosc)
	{
		case WLACZ_REFL:
			glEnable(GL_LIGHT0);
			break;
		case WYLACZ_REFL:
			glDisable(GL_LIGHT0);
			break;
		case WLACZ_KIER:
			glEnable(GL_LIGHT1);
			break;
		case WYLACZ_KIER:
			glDisable(GL_LIGHT1);
			break;
		case EXIT:
			exit(0);
			break;
	}
}

void menuSkala(int wartosc)
{
	switch(wartosc)
	{
		case SKALA_SZARA:
			jakaSkala = SKALA_SZARA;
			break;
		case SKALA_KOLOR:
			jakaSkala = SKALA_KOLOR;
			break;
		case SKALA_SIATKA:
			jakaSkala = SKALA_SIATKA;
			break;
		case EXIT:
			exit(0);
			break;
	}
}
