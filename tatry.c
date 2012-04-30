/* Pliki nagłówkowe */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 

#include <math.h>
#include <time.h>

#include <gl/gl.h> 
#include <gl/glut.h>


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


/* Nagłówki funkcji */

void resetKamery(void);

void mysz       (int button, int state, int x, int y);	// Obsługa myszy
void mysz_ruch  (int x, int y);							// Obsługa ruchu myszy

void klawisz    (GLubyte k, int x, int y);				// Obsługa klawiszy

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

	glutDisplayFunc(rysuj);			// def. funkcji rysującej
    glutIdleFunc(rysuj);			// def. funkcji rysującej w czasie wolnym procesoora (w efekcie: ciągle wykonywanej)
    glutReshapeFunc(rozmiar);		// def. obsługi zdarzenia resize (GLUT)
	
	glutKeyboardFunc(klawisz);		// def. obsługi klawiatury
    glutMouseFunc(mysz); 			// def. obsługi zdarzenia przycisku myszy (GLUT)
	glutMotionFunc(mysz_ruch); 		// def. obsługi zdarzenia ruchu myszy (GLUT)
	
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
    GLfloat size[2];  		//dopuszczalny zakres wielkości
    GLfloat step; 			//przyrost rozmiaru
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();	
	
	// Tło
	glClearColor(0.1, 0.1, 0.1, 0.0);		
	
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//glEnable(GL_COLOR_MATERIAL); 
	
	glTranslatef(0.0, 0.0, -1000);
	
	
	/* --- Myszka i kamera --- */
	
	// Obsługa myszką sceny
    kameraKat += kameraPredkoscObrotu;		
	kameraX   += kameraPredkosc * sin(kameraKat);
    kameraZ   -= kameraPredkosc * cos(kameraKat);
	
	// Kamera
	gluLookAt(kameraX, 0, kameraZ, kameraX + 100 * sin(kameraKat), 0, kameraZ - 100*cos(kameraKat), 0, 1, 0); 
	
	// Pobranie dopuszczalnych rozmiarów punktów
    glGetFloatv(GL_POINT_SIZE_RANGE, size);
    glGetFloatv(GL_POINT_SIZE_GRANULARITY, &step);
    
	// Ustalenie początkowego rozmiaru
    curSize=size[0];
	
	
	/* --- Rysowanie i "kolorowanie" --- */
	
	glRotatef(dx, 1.0, 0.0, 0.0);
    glRotatef(dy, 0.0, 1.0, 0.0);
    glRotatef(dz, 0.0, 0.0, 1.0);
	
	
	for(k = 0; k < iloscLinii; k ++)
	{
		//glColor3f(1.0-(float)(1.0/6.0)*fabs(tablicaDaneZ[k]), 0.0+(float)(1.0/6.0)*fabs(tablicaDaneZ[k]), 0.0+(float)(1.0/6.0)*fabs(tablicaDaneZ[k]));
		glColor3f(1.0, 0.0, 0.0);
          
		glBegin(GL_TRIANGLES);
          
		glVertex3f(tablicaDaneX[k], tablicaDaneY[k], tablicaDaneZ[k]);
		glVertex3f(tablicaDaneX[k], tablicaDaneY[k+1], tablicaDaneZ[k]);    
		glVertex3f(tablicaDaneX[k+1], tablicaDaneY[k], tablicaDaneZ[k]);
          
		glEnd();
	}
	
	
	
	/* ---------------------- */
  
    //glDisable(GL_COLOR_MATERIAL);
	
    glFlush(); 				// Rysowanie w buforze
    glutSwapBuffers(); 		// Wysłanie na ekran
}

void wczytajDane(void)
{
	int k = 0;
	FILE* f;
	
	if((f = fopen(nazwaPliku, "r")) == NULL)
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
		
	rewind(f);			// Przewinięcie na początek pliku
	
	return ile;
}

void alokujTablice(int rozmiar)
{
	tablicaDaneX = (GLfloat*)malloc(rozmiar * sizeof(GLfloat));
	tablicaDaneY = (GLfloat*)malloc(rozmiar * sizeof(GLfloat));
	tablicaDaneZ = (GLfloat*)malloc(rozmiar * sizeof(GLfloat));
}

void zwolnijPamiec(void)
{
	free(tablicaDaneX);
	free(tablicaDaneY);
	free(tablicaDaneZ);
}
