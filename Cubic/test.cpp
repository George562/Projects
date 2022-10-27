#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL\glut.h>
 
 
// угол поворота камеры
float angle=0.0;
// координаты вектора направления движения камеры
float lx=0.0f, lz=-1.0f;
// XZ позиция камеры
float x=0.0f, z=5.0f;
//Ключи статуса камеры. Переменные инициализируются нулевыми значениями
//когда клавиши не нажаты
float deltaAngle = 0.0f;
float deltaMove = 0;
int xOrigin = -1;
 
// Определения констант для меню
#define RED 1
#define GREEN 2
#define BLUE 3
#define ORANGE 4
 
#define FILL 1
#define LINE 2
 
// идентификаторы меню
int fillMenu, fontMenu, mainMenu, colorMenu;
//цвет носа
float red = 1.0f, blue=0.5f, green=0.5f;
//размер снеговика
float scale = 1.0f;
//статус меню
int menuFlag = 0;
 
// шрифт по умолчанию
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
 
#define INT_GLUT_BITMAP_8_BY_13			1
#define INT_GLUT_BITMAP_9_BY_15			2
#define INT_GLUT_BITMAP_TIMES_ROMAN_10  3
#define INT_GLUT_BITMAP_TIMES_ROMAN_24  4
#define INT_GLUT_BITMAP_HELVETICA_10	5
#define INT_GLUT_BITMAP_HELVETICA_12	6
#define INT_GLUT_BITMAP_HELVETICA_18	7
 
void changeSize(int w, int h) {
	// предотвращение деления на ноль
	if (h == 0)
		h = 1;
	float ratio =  w * 1.0 / h;
	// используем матрицу проекции
	glMatrixMode(GL_PROJECTION);
	// обнуляем матрицу
	glLoadIdentity();
	// установить параметры вьюпорта
	glViewport(0, 0, w, h);
	// установить корректную перспективу
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);
	// вернуться к матрице проекции
	glMatrixMode(GL_MODELVIEW);
}
 
void drawSnowMan() {
	glScalef(scale, scale, scale);
	glColor3f(1.0f, 1.0f, 1.0f);
	// тело снеговика
	glTranslatef(0.0f ,0.75f, 0.0f);
	glutSolidSphere(0.75f,20,20);
	// голова снеговика
	glTranslatef(0.0f, 1.0f, 0.0f);
	glutSolidSphere(0.25f,20,20);
	// глаза снеговика
	glPushMatrix();
	glColor3f(0.0f,0.0f,0.0f);
	glTranslatef(0.05f, 0.10f, 0.18f);
	glutSolidSphere(0.05f,10,10);
	glTranslatef(-0.1f, 0.0f, 0.0f);
	glutSolidSphere(0.05f,10,10);
	glPopMatrix();
	// нос снеговика
	glColor3f(red, green, blue);
	glRotatef(0.0f,1.0f, 0.0f, 0.0f);
	glutSolidCone(0.08f,0.5f,10,2);
 
	glColor3f(1.0f, 1.0f, 1.0f);
 
}
 
void renderBitmapString(
		float x,
		float y,
		float z,
		void *font,
		char *string) {
  char *c;
  glRasterPos3f(x, y,z);
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}
 
void computePos(float deltaMove) {
 
	x += deltaMove * lx * 0.1f;
	z += deltaMove * lz * 0.1f;
}
 
void renderScene(void) {
	if (deltaMove)
		computePos(deltaMove);
	//очистить буфер цвета и глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// обнулить трансформацию
	glLoadIdentity();
	// установить камеру
	gluLookAt(	   x,	1.0f,	  z,
				x+lx,	1.0f,  z+lz,
				0.0f,   1.0f,  0.0f		);
	// нарисуем "землю"
	glColor3f(0.9f, 0.9f, 0.9f);
	// полигон (plaine)
	glBegin(GL_QUADS);
		glVertex3f(-100.0f, 0.0f, -100.0f);
		glVertex3f(-100.0f, 0.0f,  100.0f);
		glVertex3f( 100.0f, 0.0f,  100.0f);
		glVertex3f( 100.0f, 0.0f, -100.0f);
	glEnd();
	// Нарисуем 64 снеговика
	char number[4];
	for(int i = -4; i < 4; i++)
		for(int j=-4; j < 4; j++) {
			glPushMatrix();
			glTranslatef(i*10.0f, 0.0f, j * 10.0f);
			drawSnowMan();
			sprintf(number,"%d",(i+4)*8+(j+4));
			renderBitmapString(0.0f, 0.5f, 0.0f, (void *)font ,number);
			glPopMatrix();
		}
 
	glutSwapBuffers();
}
// -----------------------------------	//
//            клавиатура				//
// -----------------------------------	//
void processNormalKeys(unsigned char key, int xx, int yy) {
	switch (key) {
		case 27:
			glutDestroyMenu(mainMenu);
			glutDestroyMenu(fillMenu);
			glutDestroyMenu(colorMenu);
			glutDestroyMenu(fontMenu);
			exit(0);
			break;
	}
}
 
void pressKey(int key, int xx, int yy) {
	switch (key) {
		case GLUT_KEY_UP : deltaMove = 0.5f; break;
		case GLUT_KEY_DOWN : deltaMove = -0.5f; break;
	}
}
 
void releaseKey(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP :
		case GLUT_KEY_DOWN : deltaMove = 0;break;
	}
}
// -----------------------------------	//
//            функции мыши				//
// -----------------------------------	//
void mouseMove(int x, int y) {
 
	// только когда левая кнопка не активна
	if (xOrigin >= 0) {
 
		// обновить deltaAngle
		deltaAngle = (x - xOrigin) * 0.001f;
 
		// обновить направление камеры
		lx = sin(angle + deltaAngle);
		lz = -cos(angle + deltaAngle);
	}
}
 
void mouseButton(int button, int state, int x, int y) {
	// только начало движение, если левая кнопка мыши нажата
	if (button == GLUT_LEFT_BUTTON) {
		// когда кнопка отпущена
		if (state == GLUT_UP) {
			angle += deltaAngle;
			xOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			xOrigin = x;
		}
	}
}
 
// ------------------------------------//
//            меню  					//
// ------------------------------------//
void processMenuStatus(int status, int x, int y) {
 
	if (status == GLUT_MENU_IN_USE)
		menuFlag = 1;
	else
		menuFlag = 0;
}
 
void processMainMenu(int option) {
 
	//ничего здесь не делаем
	//все действия для подменю
}
 
void processFillMenu(int option) {
	switch (option) {
		case FILL: glPolygonMode(GL_FRONT, GL_FILL); break;
		case LINE: glPolygonMode(GL_FRONT, GL_LINE); break;
	}
}
 
void processFontMenu(int option) {
	switch (option) {
		case INT_GLUT_BITMAP_8_BY_13:
			font = GLUT_BITMAP_8_BY_13;
			break;
		case INT_GLUT_BITMAP_9_BY_15:
			font = GLUT_BITMAP_9_BY_15;
			break;
		case INT_GLUT_BITMAP_TIMES_ROMAN_10:
			font = GLUT_BITMAP_TIMES_ROMAN_10;
			break;
		case INT_GLUT_BITMAP_TIMES_ROMAN_24:
			font = GLUT_BITMAP_TIMES_ROMAN_24;
			break;
		case INT_GLUT_BITMAP_HELVETICA_10:
			font = GLUT_BITMAP_HELVETICA_10;
			break;
		case INT_GLUT_BITMAP_HELVETICA_12:
			font = GLUT_BITMAP_HELVETICA_12;
			break;
		case INT_GLUT_BITMAP_HELVETICA_18:
			font = GLUT_BITMAP_HELVETICA_18;
			break;
	}
}
 
void processColorMenu(int option) {
	switch (option) {
		case RED :
			red = 1.0f;
			green = 0.0f;
			blue = 0.0f; break;
		case GREEN :
			red = 0.0f;
			green = 1.0f;
			blue = 0.0f; break;
		case BLUE :
			red = 0.0f;
			green = 0.0f;
			blue = 1.0f; break;
		case ORANGE :
			red = 1.0f;
			green = 0.5f;
			blue = 0.5f; break;
	}
}
 
void createPopupMenus() {
	fontMenu = glutCreateMenu(processFontMenu);
	glutAddMenuEntry("BITMAP_8_BY_13 ",INT_GLUT_BITMAP_8_BY_13 );
	glutAddMenuEntry("BITMAP_9_BY_15",INT_GLUT_BITMAP_9_BY_15 );
	glutAddMenuEntry("BITMAP_TIMES_ROMAN_10 ",INT_GLUT_BITMAP_TIMES_ROMAN_10  );
	glutAddMenuEntry("BITMAP_TIMES_ROMAN_24",INT_GLUT_BITMAP_TIMES_ROMAN_24  );
	glutAddMenuEntry("BITMAP_HELVETICA_10 ",INT_GLUT_BITMAP_HELVETICA_10  );
	glutAddMenuEntry("BITMAP_HELVETICA_12",INT_GLUT_BITMAP_HELVETICA_12  );
	glutAddMenuEntry("BITMAP_HELVETICA_18",INT_GLUT_BITMAP_HELVETICA_18  );
	fillMenu = glutCreateMenu(processFillMenu);
	glutAddMenuEntry("Fill",FILL);
	glutAddMenuEntry("Line",LINE);
	colorMenu = glutCreateMenu(processColorMenu);
	glutAddMenuEntry("Red",RED);
	glutAddMenuEntry("Blue",BLUE);
	glutAddMenuEntry("Green",GREEN);
	glutAddMenuEntry("Orange",ORANGE);
	mainMenu = glutCreateMenu(processMainMenu);
	glutAddSubMenu("Polygon Mode", fillMenu);
	glutAddSubMenu("Color", colorMenu);
	glutAddSubMenu("Font",fontMenu);
	// прикрепить меню к правой кнопке
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//статус активности меню
	glutMenuStatusFunc(processMenuStatus);
}
// ------------------------------------	//
//             main()					//
// -----------------------------------	//
int main(int argc, char **argv) {
	// инициализация Glut и создание окна
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(400,400);
	glutCreateWindow("Урок 11");
	// регистрация
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(pressKey);
	glutSpecialUpFunc(releaseKey);
	// две новые функции
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	// OpenGL инициализация
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// инициализация меню
	createPopupMenus();
	// главный цикл
	glutMainLoop();
	return 1;
}