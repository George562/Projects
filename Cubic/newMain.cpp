#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <Gl/glut.h>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <fstream>
// -lopengl32 -lgdi32 -lfreeglut -lglu32
using str = std::string;
using pii = std::pair<int, int>;
using vf = std::vector<float>;
using vvf = std::vector<std::vector<float>>;

// параметр количесва случайных поворотов
// обучалка

int random(int min, int max) { return abs(rand() % (max - min + 1)) + min; } // случайное число в диапазоне [min, max]

std::queue<pii> QofKeys; // очередь комманд

void processNormalKeys(unsigned char symbol, int xx, int yy);
void pressKey(int key0, int xx, int yy);

const int scw = 1200, sch = 800; 	// ширина окна и  высота окна
float ax = 20, ay = -15, az = 0.0;  // углы поворота (начальное)
const int kV = 8;  					// число вершин
int const kG = 6; 					// число граней   (обход против часовой)
const float a = 1;					// 1/2 стороны кубика  
int kf = 180;						// число шагов для поворота на 90 гр.
double df0 = 90.f / kf;				// шаг при повороте

//статус меню
int menuFlag = 0;

// идентификаторы меню
int SpeedMenu, mainMenu;

// координаты вектора направления движения камеры
float lx = 0.0f, ly = 0.0f, lz = 0.0f;
// XZ позиция камеры
float x = 9.0f, y = 7.f, z = 15.0f;
//Ключи статуса камеры. Переменные инициализируются нулевыми значениями
//когда клавиши не нажаты
float deltaAngle = 0.0f;
float deltaMove = 0;
int xOrigin = -1;

void *font = GLUT_BITMAP_TIMES_ROMAN_24;

GLfloat aV[kV][3] = { {a,-a,-a}, {-a,-a,-a}, {-a,-a, a}, {a,-a, a},   // нижние вершины
					  {a, a,-a}, {-a, a,-a}, {-a, a, a}, {a, a, a} };  // верхние
int mG[kG][4] = { {0, 3, 2, 1}, {4, 5, 6, 7}, {2, 3, 7, 6},   // 0-нижн.грань, 1-верхняя, 2-передняя
				  {0, 4, 7, 3}, {0, 1, 5, 4}, {1, 2, 6, 5} }; // 3-правая, 4-задняя, 5-левая

float mColor0[kG][3] = { {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, {1.0, 0.0, 0.0},
				  	     {0.0, 0.0, 1.0}, {1.0, 0.4, 0.0}, {0.0, 1.0, 0.0} };
std::map<vf, int> mappingColors {
	{vf(mColor0[0], mColor0[0] + 3), 0},
	{vf(mColor0[1], mColor0[1] + 3), 1},
	{vf(mColor0[2], mColor0[2] + 3), 2},
	{vf(mColor0[3], mColor0[3] + 3), 3},
	{vf(mColor0[4], mColor0[4] + 3), 4},
	{vf(mColor0[5], mColor0[5] + 3), 5}
};
struct cubic {
	float x, y, z; // координаты центра
	// цвета циклически меняются после поворота
	float mCol[kG][3]; //= { {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, {1.0, 0.0, 0.0},
					   //{0.0, 0.0, 1.0}, {1.0, 0.4, 0.0}, {0.0, 1.0, 0.0} };
};
/// 0 нижняя грань желтая 		1 верхняя - белая
 // 2 передняя грань красная	3 правая боковая синяя
 // 4 задняя грань оранжевая 	5 левая грань зеленая

int const kk = 27;  // количество кубиков
cubic mcub[kk];  //  малые кубики

//------------------------------------------------------
const int kp = 10;
int mp[kp][9] = { // порядок смены позиции кубиков при повороте
	{'f',  2,  5,  8, 17, 26, 23, 20, 11},  // F 2 -> 5 -> 8 ... 20->11 11->2
	{'g',  2, 11, 20, 23, 26, 17,  8,  5},  // G
	{'u',  8,  7,  6, 15, 24, 25, 26, 17},  // U
	{'y',  8, 17, 26, 25, 24, 15,  6,  7},  // Y
	{'d',  2, 11, 20, 19, 18,  9,  0,  1},  // D
	{'s',  1,  0,  9, 18, 19, 20, 11,  2},  // S
	{'r', 18, 19, 20, 23, 26, 25, 24, 21},  // R
	{'e', 18, 21, 24, 25, 26, 23, 20, 19},  // E
	{'l',  0,  3,  6,  7,  8,  5,  2,  1},  // L
	{'k',  0,  1,  2,  5,  8,  7,  6,  3}   // K
};

int mr[kp][5] = { // порядок смены граней при повороте
	{'f', 1, 3, 0, 5},  // F 1 -> 3 -> 0 -> 5 -> 1
	{'g', 1, 5, 0, 3},  // G
	{'y', 2, 3, 4, 5},  // U
	{'u', 2, 5, 4, 3},  // Y
	{'r', 1, 4, 0, 2},  // R
	{'e', 1, 2, 0, 4},  // E
	{'l', 1, 2, 0, 4},  // L
	{'k', 1, 4, 0, 2},  // K
	{'d', 2, 3, 4, 5},  // D
	{'s', 2, 5, 4, 3}   // S
};


// функции работы с гранями
float mg[3]; // врем.
void saveG(int j, int i);
void loadG(int j, int i);
void setG(int j, int from, int to);
void rotG(int j, unsigned char key);
//------------------------------------------------

// функции работы с гранями и их поворота
float mC[kG][3]; // врем.
void saveP(int j);
void loadP(int j);
void setP(int j1, int j2);
void rotP(unsigned char key);
void rotAll(int key);

// ----------------------------------------------------------  
void initv(); // инициализация кубиков
void showG(int j, int i1, int i2, int i3, int i4, int k0); // отрисовка грани j-го кубика в цвете k0
void showCube(); // при повороте всего

float bx = 0, by = 0, bz = 0;
double df = 0;

void showCubeG(unsigned char key); // при повороте  грани
int mk[6] = { 0, 0, 0, 0, 0, 0 };

void kcol(int j, int ng); // номер кубика, номер грани

bool contr(); // просмотр внешних граней

std::vector<str> commands { "UP",  "DN",  "RI", "LE",  "PU",  "PD",  "F",  "G",  "U",  "Y",  "R",  "E",  "L",  "K",  "D",  "S" };
std::map<str, pii> keys {
    {"UP", {1, GLUT_KEY_UP}}, {"DN", {1, GLUT_KEY_DOWN}}, {"RI", {1, GLUT_KEY_RIGHT}},
    {"LE", {1, GLUT_KEY_LEFT}}, {"PU", {1, GLUT_KEY_PAGE_UP}}, {"PD", {1, GLUT_KEY_PAGE_DOWN}},
    {"F", {0, 'f'}}, {"G", {0, 'g'}}, {"U", {0, 'u'}}, {"Y", {0, 'y'}}, {"R", {0, 'r'}}, {"E", {0, 'e'}},
    {"L", {0, 'l'}}, {"K", {0, 'k'}}, {"D", {0, 'd'}}, {"S", {0, 's'}}
};
std::map<pii, str> pairToKey {
    {{1, GLUT_KEY_UP}, "UP"}, {{1, GLUT_KEY_DOWN}, "DN"}, {{1, GLUT_KEY_RIGHT}, "RI"},
    {{1, GLUT_KEY_LEFT}, "LE"}, {{1, GLUT_KEY_PAGE_UP}, "PU"}, {{1, GLUT_KEY_PAGE_DOWN}, "PD"},
    {{0, 'f'}, "F"}, {{0, 'g'}, "G"}, {{0, 'u'}, "U"}, {{0, 'y'}, "Y"}, {{0, 'r'}, "R"}, {{0, 'e'}, "E"},
    {{0, 'l'}, "L"}, {{0, 'k'}, "K"}, {{0, 'd'}, "D"}, {{0, 's'}, "S"}
};
std::vector<std::vector<pii>> combins; // комбинации комманд { { {0, 'd'}, {0, 'r'} }, { {0, 'e'}, {0, 's'} } }

unsigned char key = 0;
int key0 = 0;
int k = 0, dk = 0;
bool IsRotateAll = true; // поворот всего

bool setKey(unsigned char key); // обработка нажания клавиши

void shuffle();

void loadCombins();
str makeStrComm(int i);
void doCombin(int i);
void SaveCube();
void LoadCube();


void changeSize(int w, int h) {
	// предотвращение деления на ноль
	if (h == 0) h = 1;
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
 
void renderBitmapString(float x, float y, float z, void *font, str string) {
    glRasterPos3f(x, y,z);
    for (char c: string)
        glutBitmapCharacter(font, c);
}
 
void renderScene(void) {
	//очистить буфер цвета и глубины
    glClearColor(0.6f, 0.6f, 0.6f, 0.6f); // серый фон
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// обнулить трансформацию
	glLoadIdentity();
	// установить камеру
	gluLookAt( x + lx, y + ly, z + lz,
				   lx,     ly,     lz,
				 0.0f,   1.0f,   0.0f );

	if (k == 0) {
        if (!QofKeys.empty()) {
            if (QofKeys.front().first == 0)
                processNormalKeys(QofKeys.front().second, 0, 0);
            else
                pressKey(QofKeys.front().second, 0, 0);
            QofKeys.pop();
        }
        showCube();
    }
    if (k > 0) { // промежуточное состояние (вращение)
        if (IsRotateAll) { // поворот всего
            glPushMatrix();
            glRotatef(k*df, bx, by, bz);  //Поворот всего вокруг 0x, 0y или 0z 
            showCube();			// показать кубик
            glPopMatrix();
        } else { // поворот грани
            glPushMatrix();
            glRotatef(k*df, bx, by, bz);
            showCubeG(key);			// показать кубик
        }
    }
    k += dk; 
    if (k > kf) { // поворот закончен 
        k = 0; dk = 0;
        if (IsRotateAll) rotAll(key0);
        else rotP(key);
        bool err = contr(); // проверка корректности граней
        if (!err)
            err = false;
    }

    glColor3f(0.F, 0.F, 0.F);
    renderBitmapString((x + lx) * 0.5, (y + ly) * 0.5 + 1.5, (z + lz) * 0.5 - 15, (void *)font, "Esc - exit");
    renderBitmapString((x + lx) * 0.5, (y + ly) * 0.5 + 0.5, (z + lz) * 0.5 - 15, (void *)font, "Space - solve it");
    renderBitmapString((x + lx) * 0.5, (y + ly) * 0.5 - 0.5, (z + lz) * 0.5 - 15, (void *)font, "Enter - shuffle 20 times");
    for (int i = 1; i <= combins.size(); i++)
        renderBitmapString((x + lx) * 0.5, (y + ly) * 0.5 - 0.5 - i, (z + lz) * 0.5 - 15, (void *)font, makeStrComm(i));
	
	glutSwapBuffers();
}

// -----------------------------------	//
//            клавиатура				//
// -----------------------------------	//
void processNormalKeys(unsigned char symbol, int xx, int yy) {
    if (k != 0) {
        if (QofKeys.size() < 3)
            QofKeys.push({0, symbol}); // 0 - unsigned char
        return;
    }
    bx = by = bz = 0;
	dk = 1; IsRotateAll = false;
    if ('1' <= symbol && symbol <= '9') doCombin(symbol - '0');
	switch (symbol) {
		case 'f': { df = -df0; bz = 1; break; } // F
		case 'g': { df = df0;  bz = 1; break; } // G
		case 'u': { df = -df0; by = 1; break; } // U
		case 'y': { df = df0;  by = 1; break; } // Y
		case 'r': { df = -df0; bx = 1; break; } // R
		case 'e': { df = df0;  bx = 1; break; } // E
		case 'l': { df = df0;  bx = 1; break; } // L
		case 'k': { df = -df0; bx = 1; break; } // K
		case 'd': { df = df0;  by = 1; break; } // D
		case 's': { df = -df0; by = 1; break; } // S
		case ' ': { initv();           break; } // Space
		case 13:  { shuffle();         break; } // Enter
		case 27:  { glutDestroyMenu(mainMenu);
					glutDestroyMenu(SpeedMenu);
					exit(0);
					break; } // Esc
		default: dk = 0; //  к обработке следующего событи
	}
    key = symbol;
	SaveCube();
}

void pressKey(int key, int xx, int yy) {
    if (k != 0) {
        if (QofKeys.size() < 3)
            QofKeys.push({1, key}); // 1 - int
        return;
    }
    bx = 0, by = 0, bz = 0;
	dk = 1; IsRotateAll = true;
	switch (key) {
		case GLUT_KEY_UP: 		 { df = -df0; bx = 1; break; } // стрелка вверх: передняя становится верхней
		case GLUT_KEY_DOWN: 	 { df = df0;  bx = 1; break; } // стрелка вниз верхняя становится предней
		case GLUT_KEY_RIGHT: 	 { df = df0;  by = 1; break; } // стрелка вправо
		case GLUT_KEY_LEFT: 	 { df = -df0; by = 1; break; } // стрелка влево
		case GLUT_KEY_PAGE_UP: 	 { df = df0;  bz = 1; break; } // PgUp: поворот против часовой стрелки
		case GLUT_KEY_PAGE_DOWN: { df = -df0; bz = 1; break; } // PgDn: поворот по часовой стрелке
		default: 				 { dk = 0; bx = by = bz = 0; } //  к обработке следующего событи
	}
	key0 = key;
	SaveCube();
}

// ------------------------------------//
//            меню  				   //
// ------------------------------------//

enum Speeds { slow = 1, medium, fast, express }; // скорость поворота грани при выполнении комманд

void processMenuStatus(int status, int x, int y) {
	menuFlag = ((status == GLUT_MENU_IN_USE) ? 1 : 0);
}
 
void processMainMenu(int option) {
 
	//ничего здесь не делаем
	//все действия для подменю
}
 
void processSpeedMenu(int option) {
	switch (option) {
		case slow:
			kf = 360;
			df0 = 90.f / kf;
			break;
		case medium:
			kf = 180;
			df0 = 90.f / kf;
			break;
		case fast:
			kf = 90;
			df0 = 90.f / kf;
			break;
		case express:
			kf = 45;
			df0 = 90.f / kf;
			break;
	}
}

void createPopupMenus() {
	SpeedMenu = glutCreateMenu(processSpeedMenu);
	glutAddMenuEntry("slow ", slow);
	glutAddMenuEntry("medium ", medium);
	glutAddMenuEntry("fast ", fast);
	glutAddMenuEntry("express ", express);
	mainMenu = glutCreateMenu(processMainMenu);
	glutAddSubMenu("SpeedMenu", SpeedMenu);
	// прикрепить меню к правой кнопке
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//статус активности меню
	glutMenuStatusFunc(processMenuStatus);
}

// ------------------------------------	//
//             main()					//
// -----------------------------------	//
int main(int argc, char **argv) {
    initv();
	LoadCube();
    loadCombins();
	// инициализация Glut и создание окна
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(scw, sch);
	glutCreateWindow("Cтрелки PgUp PgDn (F G) (L K) (U Y) (R E) (D S)");
	// регистрация
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(pressKey);
	// OpenGL инициализация
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// инициализация меню
	createPopupMenus();
	// главный цикл
	glutMainLoop();
	SaveCube(); 
	return 1;
}

// функции работы с гранями
void saveG(int j, int i) { // i - номер грани
	for (int k = 0; k < 3; k++) mg[k] = mcub[j].mCol[i][k];
}
void loadG(int j, int i) {
	for (int k = 0; k < 3; k++) mcub[j].mCol[i][k] = mg[k];
}
void setG(int j, int from, int to) { // i1 <- i2
	for (int k = 0; k < 3; k++) mcub[j].mCol[from][k] = mcub[j].mCol[to][k];
}
void rotG(int j, unsigned char key) { // цикл.смена граней (цветов) j-кубика при повороте по key
//	{70, 1, 3, 0, 5}  // F 1 -> 3 -> 0 -> 5 -> 1
	int i0 = -1; for (int i = 0; i < kp; i++) if (key == mr[i][0]) { i0 = i; break; }
	if (i0 < 0) return;
	saveG(j, mr[i0][4]);
	for (int i = 4; i > 1; i--) setG(j, mr[i0][i], mr[i0][i - 1]);
	loadG(j, mr[i0][1]);
}
//------------------------------------------------

// функции работы с гранями и их поворота
void saveP(int j) {
	for (int i = 0; i < kG; i++) for(int k =0; k<3; k++) mC[i][k] = mcub[j].mCol[i][k];
}
void loadP(int j) {
	for (int i = 0; i < kG; i++) for (int k = 0; k < 3; k++) mcub[j].mCol[i][k] = mC[i][k];
}
void setP(int j1, int j2) { // цвета j1 <- цвета j2  
	for (int i = 0; i < kG; i++) {
		for (int k = 0; k < 3; k++) mcub[j1].mCol[i][k] = mcub[j2].mCol[i][k];
	}
}
void rotP(unsigned char key) { // смена 8-ми кубиков (цветов) при повороте грани
	// mp {key, 2, 5, 8, 17, 26, 23, 20, 11}  // F 2 
	int i0 = -1;
	for (int i = 0; i < kp; i++) if (key == mp[i][0]) { i0 = i; break; }
	if (i0 < 0) return;
	saveP(mp[i0][8]);
	for (int i = 8; i > 3; i-=2) setP(mp[i0][i], mp[i0][i - 2]);
	loadP(mp[i0][2]);
	saveP(mp[i0][7]);
	for (int i = 7; i > 1; i-=2) setP(mp[i0][i], mp[i0][i - 2]);
	loadP(mp[i0][1]);
	for (int i = 1; i < 9; i++)
		rotG(mp[i0][i], key); // сменить цвета граней
}
void rotAll(int key0) { // смена всех кубиков (цветов) при повороте грани
	int i0 = -1, di = 0;
	switch (key0) {
		case GLUT_KEY_UP:       { key = 'r'; break; } // стрелка вверх: передняя становится верхней и т.д.
		case GLUT_KEY_DOWN:     { key = 'e'; break; } // стрелка вниз верхняя становится предней // 1 -> 2 -> 0 -> 4 -> 1    4, 1, 2, 0
		case GLUT_KEY_RIGHT:    { key = 'y'; break; } // стрелка вправо
		case GLUT_KEY_LEFT:     { key = 'u'; break; } // стрелка влево
		case GLUT_KEY_PAGE_UP:  { key = 'g'; break; } // PgUp
		case GLUT_KEY_PAGE_DOWN:{ key = 'f'; break; } // PgDn
	}
	for (int i = 0; i < kp; i++) if (key == mp[i][0]) { i0 = i; break; }
	if (i0 < 0) return; 
	if (key == 'f' || key == 'g') di = 1;   // повороты вокрух 0y  
	if (key == 'u' || key == 'y') di = 3;   // 0y  
	if (key == 'r' || key == 'e') di = 9;   // 0x  
	// mp {70, 2, 5, 8, 17, 26, 23, 20, 11}  // F 2 -> 5 -> 8 ... 20->11 11->2
	for (int k = 0, dd = 0; k < 3; k++) { // три слоя
		saveP(mp[i0][8] - dd);
		for (int i = 8; i > 3; i-=2)
			setP(mp[i0][i] - dd, mp[i0][i - 2] - dd);
		loadP(mp[i0][2] - dd);
		saveP(mp[i0][7] - dd);
		for (int i = 7; i > 2; i-=2)
			setP(mp[i0][i] - dd, mp[i0][i - 2] - dd);
		loadP(mp[i0][1] - dd);
		for (int i = 1; i < 9; i++) {
			rotG(mp[i0][i] - dd, key); // сменить цвета граней
		}
		dd += di;
	}
}
// ----------------------------------------------------------  
void initv() { // инициализация кубиков
	for (int i = -1, n = 0; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			for (int k = -1; k <= 1; k++, n++) {
				mcub[n].x = 2 * a * i;
				mcub[n].y = 2 * a * j;
				mcub[n].z = 2 * a * k;
			}
	
	for (int i = 0; i < kk; i++) {
		for (int j = 0; j < kG; j++) {
			mcub[i].mCol[j][0] = mColor0[j][0];
			mcub[i].mCol[j][1] = mColor0[j][1];
			mcub[i].mCol[j][2] = mColor0[j][2];
		}
	}
}
void showG(int j, int i1, int i2, int i3, int i4, int k0) { // отрисовка грани j-го кубика в цвете k0
	glColor3fv(mcub[j].mCol[k0]);  // цвета граней свои!
	int x = mcub[j].x, y = mcub[j].y, z = mcub[j].z; // коорд-ты центра кубика
	glBegin(GL_QUADS); // делаем сдвиги верщин
		glVertex3f(aV[i1][0] + x, aV[i1][1] + y, aV[i1][2] + z);  
		glVertex3f(aV[i2][0] + x, aV[i2][1] + y, aV[i2][2] + z);
		glVertex3f(aV[i3][0] + x, aV[i3][1] + y, aV[i3][2] + z);
		glVertex3f(aV[i4][0] + x, aV[i4][1] + y, aV[i4][2] + z);
	glEnd();
	glColor3f(0.0, 0.0, 0.0);  // цвет рисования рёбер - черный
	glBegin(GL_LINE_LOOP); 
		glVertex3f(aV[i1][0] + x, aV[i1][1] + y, aV[i1][2] + z);
		glVertex3f(aV[i2][0] + x, aV[i2][1] + y, aV[i2][2] + z);
		glVertex3f(aV[i3][0] + x, aV[i3][1] + y, aV[i3][2] + z);
		glVertex3f(aV[i4][0] + x, aV[i4][1] + y, aV[i4][2] + z);
	glEnd();
}
void showCube() { // при повороте всего 
	for (int j = 0; j < kk; j++) { // по кубиками  с центрами в mcub[ii][0-3]
		for (int i = 0; i < kG; i++) {  // отрисовка граней: цвета mCol[0], mCol[1], ... 
			showG(j, mG[i][0], mG[i][1], mG[i][2], mG[i][3], i); // 
		}
	}
}

void showCubeG(unsigned char key) { // при повороте  грани
	for (int j = 0; j < kk; j++) { // по кубиками  с центрами в av[ii][0-3]
		if ((key == 'f' || key == 'g') && mcub[j].z !=  2 * a) continue; //  F G
		if ((key == 'u' || key == 'y') && mcub[j].y !=  2 * a) continue; //  U Y 
		if ((key == 'r' || key == 'e') && mcub[j].x !=  2 * a) continue; //  R E 
		if ((key == 'l' || key == 'k') && mcub[j].x != -2 * a) continue; //  L K 
		if ((key == 'd' || key == 's') && mcub[j].y != -2 * a) continue; //  D S 
		for (int i = 0; i < kG; i++) {  // отрисовка поворачиваемой грани : с учетом лок.поворота
			showG(j, mG[i][0], mG[i][1], mG[i][2], mG[i][3], i); // 
		}
	}
	glPopMatrix();
	for (int j = 0; j < kk; j++) { // неподвижная часть
		if ((key == 'f' || key == 'g') && mcub[j].z ==  2 * a) continue; //  F G
		if ((key == 'u' || key == 'y') && mcub[j].y ==  2 * a) continue; //  U Y 
		if ((key == 'r' || key == 'e') && mcub[j].x ==  2 * a) continue; //  R E 
		if ((key == 'l' || key == 'k') && mcub[j].x == -2 * a) continue; //  L K 
		if ((key == 'd' || key == 's') && mcub[j].y == -2 * a) continue; //  D S 
		for (int i = 0; i < kG; i++)  // отрисовка неподвижгых граней: цвета mCol[0], mCol[1], ... 
			showG(j, mG[i][0], mG[i][1], mG[i][2], mG[i][3], i); // 
	}
}

void kcol(int j, int ng) { // номер кубика, номер грани
	int res = -1; // ищем цвет
	for (int k = 0; k < kG; k++) {
		if (mcub[j].mCol[ng][0] == mColor0[k][0] 
		   && mcub[j].mCol[ng][1] == mColor0[k][1] 
			&& mcub[j].mCol[ng][2] == mColor0[k][2]) {
			res = k; break;
		}
	}
	if (res > -1) mk[res]++;
}

bool contr() { // просмотр внешних граней
	bool b = true;  
	for (int j = 0; j < 6; j++) mk[j] = 0;
	for (int j = 0; j < kk; j++) { // по кубиками
		if (mcub[j].x ==  2 * a) kcol(j, 3);
		if (mcub[j].x == -2 * a) kcol(j, 5);
		if (mcub[j].y ==  2 * a) kcol(j, 1);
		if (mcub[j].y == -2 * a) kcol(j, 0);
		if (mcub[j].z ==  2 * a) kcol(j, 2);
		if (mcub[j].z == -2 * a) kcol(j, 4);
		//{ 1.0, 1.0, 0.0 }, { 1.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0 },
		//{ 0.0, 0.0, 1.0 }, { 1.0, 0.4, 0.3 }, { 0.0, 1.0, 0.0 }
		// 0 - нижн.грань (y=-2a), 1 - верхняя (y=2a), 2 - правая (x=2a)
		// 3 - задняя (z=-2a),     4 - левая (x=-2a),  5 - передняя (z=2a)
	};
	for (int j = 0; j < 6; j++) b = b && (mk[j] == 9);
	return b;
}

void shuffle() { // генерация случайной последовательности комманд заданной длины
    for (int i = 0; i < 20; i++) {
        QofKeys.push(keys[commands[random(6, commands.size() - 1)]]);
        do renderScene();
        while (k != 0);
    }
}

void loadCombins() { // загрузка комбинаций из файла combins.txt
    std::ifstream file;
    file.open("combins.txt");
    int n; file >> n;
    str command;
    for (int i = 0, m; i < n && i < 9; i++) {
        file >> m;
        combins.push_back(std::vector<pii>(m));
        for (int j = 0; j < m; j++) {
            file >> command;
            combins[i][j] = keys[command];
        }
    }
}

str makeStrComm(int i) { // строковое представление комбинации
    str res = std::to_string(i) + " - ";
    for (pii p: combins[i - 1]) res += pairToKey[p];
    return res;
}

void doCombin(int i) { // делаем i-ю комбинацию
    for (pii p: combins[i - 1]) {
        QofKeys.push(p);
        do renderScene();
        while (k != 0);
    }
}

void SaveCube() { // сохранить состояние кубика в файл
	std::ofstream file; file.open("save.txt");
	for (int i = 0; i < kk; i++)
		for (int j = 0; j < kG; j++) {
			vf vectorColor {mcub[i].mCol[j], mcub[i].mCol[j] + 3};
			file << mappingColors[vectorColor] << ' ';
		}
}

void LoadCube() {// загрузк состояние кубика из файла
	std::ifstream file("save.txt");
	if (!file)
		SaveCube();
	else
		for (int i = 0, colorNum; i < kk; i++)
			for (int j = 0; j < kG; j++) {
				file >> colorNum;
				if (file.eof()) return;
				mcub[i].mCol[j][0] = mColor0[colorNum][0];
				mcub[i].mCol[j][1] = mColor0[colorNum][1];
				mcub[i].mCol[j][2] = mColor0[colorNum][2];
			}
}
