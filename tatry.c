/* Pliki naglowkowe */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <math.h>
#include <time.h>
#include <gl/gl.h> 
#include "glut.h"

/* Zmienne globalne */

int screen_width  = 640;
int screen_height = 480;

int pozycjaMyszyX;
int pozycjaMyszyY;

int iloscLinii;

double kameraX;
double kameraZ;
double kameraPredkosc;

double kameraKat;
double kameraPredkoscObrotu;

GLfloat dx = 1;
GLfloat dy = 1;
GLfloat dz = 1;

const char nazwaPliku[] = "tatry_dane.txt";

GLfloat* tablicaDaneX;
GLfloat* tablicaDaneY;
GLfloat* tablicaDaneZ;


/* Naglowki funkcji */

void resetKamery(void);

void mysz       (int button, int state, int x, int y);				// Obsluga myszy
void mysz_ruch  (int x, int y);							// Obsluga ruchu myszy

void klawisz    (GLubyte k, int x, int y);					// Obsluga klawiszy

void rozmiar    (int width, int height);
void rysuj      (void);

void wczytajDane  (void);
int iloscLiniiPlik(FILE*);

void alokujTablice(int);
void zwolnijPamiec(void);


/* MAIN */

int main(int argc, char** argv)
{
	glutInit(&argc, argv);    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Tatry");    

	glutDisplayFunc(rysuj);			// def. funkcji rysujacej
	glutIdleFunc(rysuj);			// def. funkcji rysujacej w czasie wolnym procesoora (w efekcie: ciagle wykonywanej)
	glutReshapeFunc(rozmiar);		// def. obslugi zdarzenia resize (GLUT)
	
	glutKeyboardFunc(klawisz);		// def. obslugi klawiatury
    	glutMouseFunc(mysz); 			// def. obslugi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch); 		// def. obslugi zdarzenia ruchu myszy (GLUT)
	
	resetKamery();
	wczytajDane();
	
	
	glEnable(GL_DEPTH_TEST); 
	
	glutMainLoop();	

	zwolnijPamiec();

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
	int k;
	
	GLfloat curSize;  		//aktualny rozmiar
	GLfloat size[2];  		//dopuszczalny zakres wielko�ci
	GLfloat step; 			//przyrost rozmiaru
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	
	// Tlo
	glClearColor(0.1, 0.1, 0.1, 0.0);		
	
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//glEnable(GL_COLOR_MATERIAL); 
	
	glTranslatef(-19, -1, -1000);
	
	
	/* --- Myszka i kamera --- */
	
	// Obsluga myszk� sceny
	kameraKat += kameraPredkoscObrotu;		
	kameraX   += kameraPredkosc * sin(kameraKat);
	kameraZ   -= kameraPredkosc * cos(kameraKat);
	
	// Kamera
	gluLookAt(kameraX, 0, kameraZ, kameraX + 100 * sin(kameraKat), 0, kameraZ - 100*cos(kameraKat), 0, 1, 0); 
	
	// Pobranie dopuszczalnych rozmiarow punktow
	glGetFloatv(GL_POINT_SIZE_RANGE, size);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &step);
    
	// Ustalenie poczatkowego rozmiaru
	curSize=size[0];
	
	
	/* --- Rysowanie i "kolorowanie" --- */
	
	glRotatef(dx, 1.0, 0.0, 0.0);
	glRotatef(dy, 0.0, 1.0, 0.0);
	glRotatef(dz, 0.0, 0.0, 1.0);
	
	glBegin(GL_TRIANGLE_STRIP);	
	
	for(k = 0; k < iloscLinii; k++)
	{
		//glColor3f(1.0-(float)(1.0/6.0)*fabs(tablicaDaneX[k]), 0.0+(float)(1.0/6.0)*fabs(tablicaDaneY[k]), 0.0+(float)(1.0/6.0)*fabs(tablicaDaneZ[k]));
	    glColor3f(1.0, 0.0, 0.0);
          

            // for each vertex, we calculate
            // the grayscale shade color, 
            // we set the texture coordinate,
            // and we draw the vertex.

            // draw vertex 0
            glVertex3f(tablicaDaneX[k], 
                       tablicaDaneZ[k], tablicaDaneZ[k]);
            // draw vertex 1
            glVertex3f(tablicaDaneX[k+1], 
                       tablicaDaneZ[k], tablicaDaneZ[k]);
            // draw vertex 2
            glVertex3f(tablicaDaneX[k], 
                       tablicaDaneZ[k+1], tablicaDaneZ[k]);
            // draw vertex 3;
            glVertex3f(tablicaDaneX[k+1], 
                       tablicaDaneZ[k+1], tablicaDaneZ[k]);
        }
	
	glEnd();
		
	/*	GLfloat x,y,z;
		
  for(y=10 ;y >=-10;y-=1)
    {
         for(x=10;x>=-10;x-=1)
         {
              glColor3f(1.0-(float)(1.0/10.0)*fabs(x), 0.0+(float)(1.0/10.0)*fabs(y), 0.0+(float)(1.0/6)*fabs(z));
              glBegin(GL_TRIANGLES);
              z=4-(sin(sqrt(x*x+y*y)-3)+sqrt(x*x+y*y)-3);
              glVertex3f(x,y,z);
              z=4-(sin(sqrt((x+1)*(x+1)+y*y)-3)+sqrt((x+1)*(x+1)+y*y)-3);
              glVertex3f(x+1,y,z);
              z=4-(sin(sqrt(x*x+(y+1)*(y+1))-3)+sqrt(x*x+(y+1)*(y+1))-3);
              glVertex3f(x,y+1,z);
              glEnd();
         }
    }*/
	
	
	
	/* ---------------------- */
  
    //glDisable(GL_COLOR_MATERIAL);
	
    glFlush(); 				// Rysowanie w buforze
    glutSwapBuffers(); 		// Wys�anie na ekran
}

void wczytajDane(void)
{
	int k = 0;
	FILE* f;

	if((f = fopen("tatry.txt", "r")) == NULL)
	{
		printf("\nDEBUG: File error\n");
		exit(-1);
	}
	
	iloscLinii = iloscLiniiPlik(f);
	
	alokujTablice(iloscLinii);
	
	for(k = 0; k < iloscLinii; k++)
	{
		fscanf(f, "%f%f%f", &tablicaDaneX[k], &tablicaDaneY[k], &tablicaDaneZ[k]);
	}
	
	fclose(f);
}

int iloscLiniiPlik(FILE* f)
{
	char c;
	int ile = 0;

	while((c = fgetc(f)) != EOF)
		if(c == '\n')
			ile++;

	if(c != '\n')
		ile++;
		
	rewind(f);			// Przewini�cie na pocz�tek pliku
	
	return ile;
}

void alokujTablice(int rozmiar)
{
	tablicaDaneX = (float*)malloc(rozmiar * sizeof(float));
	tablicaDaneY = (float*)malloc(rozmiar * sizeof(float));
	tablicaDaneZ = (float*)malloc(rozmiar * sizeof(float));
}

void zwolnijPamiec(void)
{
	free(tablicaDaneX);
	free(tablicaDaneY);
	free(tablicaDaneZ);
}
