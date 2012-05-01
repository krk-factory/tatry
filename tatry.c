/* Pliki nagłówkowe */

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

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

//zmienne do skal
int skala=1;

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

// Maksymalne wartości poszczególnych współrzędnych
double maxX = 0;
double maxY = 0;
double maxZ = 0;

double minX = 0;
double minY = 0;
double minZ = 0;

// Nazwa pliku
const char nazwaPliku[] = "tatry_dane.txt";

// Tablica do przechowywania danych
double tablicaDane[ELEMENTY][ELEMENTY][WSPOLRZEDNE];

//enum
enum
{skala_szar,skala_kol,EXIT};

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

void menu_skala(int wartosc);


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
	int menu = glutCreateMenu(menu_skala);
	glutAddMenuEntry("skala szarosci", skala_szar);
	glutAddMenuEntry("skala kolorow", skala_kol);
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
	
	
	/* --- Rysowanie i "kolorowanie" --- */
	
	glTranslatef(-0.5, 0.3, -2.0);
	glRotatef(35, 1.0, 0.0, 0.0);
	
	glRotatef(dx, 1.0, 0.0, 0.0);
    glRotatef(dy, 0.0, 1.0, 0.0);
    glRotatef(dz, 0.0, 0.0, 1.0);
	
	glColor3f(1.0, 0.0, 0.0);
	if(skala==1)
	{
	for(k2 = 0; k2 < ELEMENTY; k2++)
		for(k1 = 0; k1 < ELEMENTY; k1++)
		{
            //glColor3f(tablicaDane[k1][k2][1],tablicaDane[k1][k2][1],tablicaDane[k1][k2][1]); 
            //glColor3f(tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1]);
            //glColor3f(tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1]);
			glBegin(GL_TRIANGLES);		// dzielimy wysokość przez 8.0, żeby górki były mniejsze i lepiej wyglądały ;-)
			{
                glColor3f(tablicaDane[k1][k2][1],tablicaDane[k1][k2][1],tablicaDane[k1][k2][1]);                     
				glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);
				
				
				glColor3f(tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1],tablicaDane[k1+1][k2][1]);
				glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);
				
				
				glColor3f(tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1],tablicaDane[k1+1][k2+1][1]);
                glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				
				//glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				//glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);
				//glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);
			}  
			glEnd();
		}
    }
    else
    {
	for(k2 = 0; k2 < ELEMENTY; k2++)
		for(k1 = 0; k1 < ELEMENTY; k1++)
		{
            glColor3f(0.3,0.3,0.3);
			glBegin(GL_TRIANGLES);		// dzielimy wysokość przez 8.0, żeby górki były mniejsze i lepiej wyglądały ;-)
			{
                glColor3f(tablicaDane[k1][k2][0],tablicaDane[k1][k2][2],tablicaDane[k1][k2][1]);                     
				glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);
				
				
				glColor3f(tablicaDane[k1+1][k2][0],tablicaDane[k1+1][k2][2],tablicaDane[k1+1][k2][1]);
				glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);
				
				
				glColor3f(tablicaDane[k1+1][k2+1][0],tablicaDane[k1+1][k2+1][2],tablicaDane[k1+1][k2+1][1]);
                glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				
				//glVertex3f(tablicaDane[k1+1][k2+1][0], tablicaDane[k1+1][k2+1][2]/8.0, tablicaDane[k1+1][k2+1][1]);
				//glVertex3f(tablicaDane[k1+1][k2][0], tablicaDane[k1+1][k2][2]/8.0, tablicaDane[k1+1][k2][1]);
				//glVertex3f(tablicaDane[k1][k2][0], tablicaDane[k1][k2][2]/8.0, tablicaDane[k1][k2][1]);
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

void menu_skala(int wartosc)
{
	switch(wartosc)
	{
		case skala_szar:
			skala=1;
			break;
		case skala_kol:
			skala=0;
			break;
		case EXIT:
			exit(0);
	}
}
