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

// обучалка
/* ToDo:
    прогресс сбора кубика:
        этапы прогресса появляются постепенно, после завершения предыдущего
        пройденные этапы выводятся зелёным текстом, текущий - жёлтым                       
        с каждым этапом открываются новые комбинации комманд
*/

enum Stage {          // этапы:
    white_cross,      //    собрать на передней грани белый крест   доступен пиф-паф
    white_face,       //    собрать белую грань                     без изменений
    second_layer,     //    собрать второй слой кубика              доступен левый пиф-паф
    last_layer_cross, //    собрать крест последнего слоя           доступен палга и галка
    last_face,        //    собрать последнюю сторону               
    set_edges,        //    расставить рёбра                        
    set_corners,      //    расставить углы                         
    done              //    кубик собран. этап не отображается пользователю
};
int curStage = white_cross;
// функции проверки выполнения стадии
bool white_cross_is_done();
bool white_face_is_done();
bool second_layer_is_done();
bool last_layer_cross_is_done();
bool last_face_is_done();
bool set_edges_is_done();
bool set_corners_is_done();
void which_stage() {                                   curStage = white_cross;
    if (white_cross_is_done()) {                       curStage = white_face;
        if (white_face_is_done()) {                    curStage = second_layer;
            if (second_layer_is_done()) {              curStage = last_layer_cross;
                if (last_layer_cross_is_done()) {      curStage = last_face;
                    if (last_face_is_done()) {         curStage = set_edges;
                        if (set_edges_is_done()) {     curStage = set_corners;
                            if (set_corners_is_done()) curStage = done;
                        }
                    }
                }
            }
        }
    }
}

int random(int min, int max) { return abs(rand() % (max - min + 1)) + min; } // случайное число в диапазоне [min, max]

std::queue<pii> QofKeys; // очередь комманд

void processNormalKeys(unsigned char symbol, int xx, int yy);
void pressKey(int key0, int xx, int yy);

const int scw = 1200, sch = 800;    // ширина окна и  высота окна
float ax = 20, ay = -15, az = 0.0;  // углы поворота (начальное)
const int kV = 8;                   // число вершин
int const kG = 6;                   // число граней   (обход против часовой)
const float a = 1;                  // 1/2 стороны кубика  
int kf = 180;                       // число шагов для поворота на 90 гр.
double df0 = 90.f / kf;             // шаг при повороте
int shuffleNumber = 20;             // количество комманд при перемешивании
str StringSpeed = "normal";         // строка для вывода на экран текущей скорости

int menuFlag = 0;           //статус меню
int mainMenu, SpeedMenu, ShuffleMenu;   // идентификаторы меню

float lx = 8.5f, ly = 4.5f, lz = 0.0f;  // координаты вектора смещения взгляда камеры
float x = 6.0f, y = 4.f, z = 20.0f;     // XYZ позиция камеры

void *font = GLUT_BITMAP_TIMES_ROMAN_24; // используемый шрифт

GLfloat aV[kV][3] = { {a,-a,-a}, {-a,-a,-a}, {-a,-a, a}, {a,-a, a},     // нижние вершины
                      {a, a,-a}, {-a, a,-a}, {-a, a, a}, {a, a, a} };   // верхние
int mG[kG][4] = { {0, 3, 2, 1}, {4, 5, 6, 7}, {2, 3, 7, 6},             // 0-нижн.грань, 1-верхняя, 2-передняя
                  {0, 4, 7, 3}, {0, 1, 5, 4}, {1, 2, 6, 5} };           // 3-правая, 4-задняя, 5-левая

float mColor0[kG][3] = { {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, {1.0, 0.0, 0.0},     // жёлтый,     белый, красный
                           {0.0, 0.0, 1.0}, {1.0, 0.4, 0.0}, {0.0, 1.0, 0.0} }; //  синий, оранжевый, зелёный
std::map<vf, int> mappingColors {
    {vf(mColor0[0], mColor0[0] + 3), 0}, // жёлтый
    {vf(mColor0[1], mColor0[1] + 3), 1}, // белый
    {vf(mColor0[2], mColor0[2] + 3), 2}, // красный
    {vf(mColor0[3], mColor0[3] + 3), 3}, // синий
    {vf(mColor0[4], mColor0[4] + 3), 4}, // оранжевый
    {vf(mColor0[5], mColor0[5] + 3), 5}  // зелёный
};
std::map<int, str> stringColors { {0, "yellow"}, {1, "white"}, {2, "red"}, {3, "blue"}, {4, "orange"}, {5, "green"} };

vf CurDirWhite {0, 1, 0, 0, 0, 0}; // на какой грани сейчас расположен белый центр
vf CurDirRed {0, 0, 1, 0, 0, 0};   // на какой грани сейчас расположен красный центр

struct cubic {
    float x, y, z; // координаты центра
    // цвета циклически меняются после поворота
    float mCol[kG][3]; //= { {1.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, {1.0, 0.0, 0.0},
                       //{0.0, 0.0, 1.0}, {1.0, 0.4, 0.0}, {0.0, 1.0, 0.0} };
};
 // 0 нижняя грань желтая       1 верхняя - белая
 // 2 передняя грань красная    3 правая боковая синяя
 // 4 задняя грань оранжевая    5 левая грань зеленая

int const kk = 27;  // количество кубиков
cubic mcub[kk];     //  малые кубики

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
void saveG(int j, int i); // i - номер грани
void loadG(int j, int i);
void setG(int j, int from, int to); // i1 <- i2
void rotG(int j, unsigned char key); // цикл.смена граней (цветов) j-кубика при повороте по key
//------------------------------------------------

// функции работы с гранями и их поворота
float mC[kG][3]; // врем.
void saveP(int j);
void loadP(int j);
void setP(int j1, int j2); // цвета j1 <- цвета j2
void rotP(unsigned char key); // смена 8-ми кубиков (цветов) при повороте грани
void rotAll(int key); // смена всех кубиков (цветов) при повороте грани

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

std::vector<str> commands { "UP", "DN", "RI", "LE", "PU", "PD", "F", "G", "U", "Y", "R", "E", "L", "K", "D", "S" };
std::map<str, pii> keys { // преобразование строкового представление ключей в пары идентификатор - комманда
    {"UP", {1, GLUT_KEY_UP}}, {"DN", {1, GLUT_KEY_DOWN}}, {"RI", {1, GLUT_KEY_RIGHT}},
    {"LE", {1, GLUT_KEY_LEFT}}, {"PU", {1, GLUT_KEY_PAGE_UP}}, {"PD", {1, GLUT_KEY_PAGE_DOWN}},
    {"F", {0, 'f'}}, {"G", {0, 'g'}}, {"U", {0, 'u'}}, {"Y", {0, 'y'}}, {"R", {0, 'r'}}, {"E", {0, 'e'}},
    {"L", {0, 'l'}}, {"K", {0, 'k'}}, {"D", {0, 'd'}}, {"S", {0, 's'}}
};
std::map<pii, str> pairToKey { // преобразование обратное тому, что представлен выше
    {{1, GLUT_KEY_UP}, "UP"}, {{1, GLUT_KEY_DOWN}, "DN"}, {{1, GLUT_KEY_RIGHT}, "RI"},
    {{1, GLUT_KEY_LEFT}, "LE"}, {{1, GLUT_KEY_PAGE_UP}, "PU"}, {{1, GLUT_KEY_PAGE_DOWN}, "PD"},
    {{0, 'f'}, "F"}, {{0, 'g'}, "G"}, {{0, 'u'}, "U"}, {{0, 'y'}, "Y"}, {{0, 'r'}, "R"}, {{0, 'e'}, "E"},
    {{0, 'l'}, "L"}, {{0, 'k'}, "K"}, {{0, 'd'}, "D"}, {{0, 's'}, "S"}
};
std::vector<std::vector<pii>> combins; // комбинации комманд { { {0, 'd'}, {0, 'r'} }, { {0, 'e'}, {0, 's'} } }

unsigned char key = 0; // ключ для комманд с идентификатором 0
int key0 = 0; // ключ для комманд с идентификатором 1
int k = 0, dk = 0;
bool IsRotateAll = true; // поворот всего

void shuffle(); // перемешивание кубика

void loadCombins(); // ключ для комманд с идентификатором 0
str makeStrComm(int i); // строковое представление комбинации
void doCombin(int i); // делаем i-ю комбинацию
void SaveCube(); // сохранить состояние кубика в файл
void LoadCube(); // загрузка состояния кубика из файла

// -----------------------------------  //
//           обработка экрана           //
// -----------------------------------  //
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
 
void renderBitmapString(float x, float y, void *font, str string) {
    glRasterPos3f(x, y, 0);
    for (char c: string)
        glutBitmapCharacter(font, c);
}
 
void renderScene(void) {
    //очистить буфер цвета и глубины
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // бежевый фон
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // обнулить трансформацию
    glLoadIdentity();
    // установить камеру
    gluLookAt( x + lx, y + ly, z + lz,
                   lx,     ly,     lz,
                 0.0f,   1.0f,   0.0f );
    
    glLineWidth(2.0f);

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
            showCube();         // показать кубик
            glPopMatrix();
        } else { // поворот грани
            glPushMatrix();
            glRotatef(k*df, bx, by, bz);
            showCubeG(key);         // показать кубик
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
        white_cross_is_done();
    }
    
    glLoadIdentity();

    gluLookAt( 0, 0, 20,
               0, 0, 0,
               0, 1, 0 );
    glColor3f(0.6F, 0.6F, 0.6F);
    glBegin(GL_QUADS);
        glVertex3f(-2, 2, -8);
        glVertex3f(-19, 2, -8);
        glVertex3f(-19, -15, -8);
        glVertex3f(-2, -15, -8);
    glEnd();

    glColor3f(0.8F, 0.8F, 0.8F);
    renderBitmapString(-12, 7, (void *)font, "Esc - exit");
    renderBitmapString(-12, 6, (void *)font, "Space - solve it");
    renderBitmapString(-12, 5, (void *)font, "Enter - shuffle " + std::to_string(shuffleNumber) + " times");
    renderBitmapString(-12, 4, (void *)font, "Current speed: " + StringSpeed);
    renderBitmapString(-12, 3, (void *)font, "Right mouse button - menu");
    renderBitmapString(0, 0, (void *)font, "Combins:");
    for (int i = 1; i <= combins.size(); i++)
        renderBitmapString(0, - i * 0.8, (void *)font, makeStrComm(i));
    

    renderBitmapString(-1, 10 * 0.7, (void *)font, "stage:");
    glColor3f(1.F, 1.F, 0.F);
    switch (curStage) {
        case done:
            glColor3f(0.F, 0.8F, 0.F);
        case set_corners:
            renderBitmapString(0, 3 * 0.7, (void *)font, "set corners");
            glColor3f(0.F, 0.8F, 0.F);
        case set_edges:
            renderBitmapString(0, 4 * 0.7, (void *)font, "set edges");
            glColor3f(0.F, 0.8F, 0.F);
        case last_face:
            renderBitmapString(0, 5 * 0.7, (void *)font, "last face");
            glColor3f(0.F, 0.8F, 0.F);
        case last_layer_cross:
            renderBitmapString(0, 6 * 0.7, (void *)font, "last layer cross");
            glColor3f(0.F, 0.8F, 0.F);
        case second_layer:
            renderBitmapString(0, 7 * 0.7, (void *)font, "second layer");
            glColor3f(0.F, 0.8F, 0.F);
        case white_face:
            renderBitmapString(0, 8 * 0.7, (void *)font, "white face");
            glColor3f(0.F, 0.8F, 0.F);
        case white_cross:
            renderBitmapString(0, 9 * 0.7, (void *)font, "white cross");
    }

    glutSwapBuffers();
}

// -----------------------------------  //
//            клавиатура                //
// -----------------------------------  //
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
                    glutDestroyMenu(ShuffleMenu);
                    exit(0);
                    break; } // Esc
        default: dk = 0; //  к обработке следующего событи
    }
    key = symbol;
    SaveCube();
    which_stage();
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
        case GLUT_KEY_UP:        { df = -df0; bx = 1; break; } // стрелка вверх: передняя становится верхней
        case GLUT_KEY_DOWN:      { df = df0;  bx = 1; break; } // стрелка вниз верхняя становится предней
        case GLUT_KEY_RIGHT:     { df = df0;  by = 1; break; } // стрелка вправо
        case GLUT_KEY_LEFT:      { df = -df0; by = 1; break; } // стрелка влево
        case GLUT_KEY_PAGE_UP:   { df = df0;  bz = 1; break; } // PgUp: поворот против часовой стрелки
        case GLUT_KEY_PAGE_DOWN: { df = -df0; bz = 1; break; } // PgDn: поворот по часовой стрелке
        default:                 { dk = 0; bx = by = bz = 0; } //  к обработке следующего событи
    }
    key0 = key;
    SaveCube();
    which_stage();
}

// ------------------------------------//
//            меню                     //
// ------------------------------------//

enum Speeds { slow = 1, medium, fast, express }; // скорость поворота грани при выполнении комманд
enum NumsOfShuffle { five = 1, ten, fifteen, twenty, twenty_five, fifty, hundred }; // скорость поворота грани при выполнении комманд

void processMenuStatus(int status, int x, int y) {
    menuFlag = ((status == GLUT_MENU_IN_USE) ? 1 : 0);
}
 
void processMainMenu(int option) {
 
    //ничего здесь не делаем
    //все действия для подменю
}
 
void processSpeedMenu(int option) { // меню выбора скорости поворота кубика
    switch (option) {
        case slow:      kf = 360;   StringSpeed = "slow";       break;
        case medium:    kf = 180;   StringSpeed = "medium";     break;
        case fast:      kf = 90;    StringSpeed = "fast";       break;
        case express:   kf = 45;    StringSpeed = "express";    break;
    }
    df0 = 90.f / kf;
}

void processShuffleMenu(int option) { // меню выбора количества комманд при перемешивании кубика
    switch (option) {
        case five:          shuffleNumber = 5;      break;
        case ten:           shuffleNumber = 10;     break;
        case fifteen:       shuffleNumber = 15;     break;
        case twenty:        shuffleNumber = 20;     break;
        case twenty_five:   shuffleNumber = 25;     break;
        case fifty:         shuffleNumber = 50;     break;
        case hundred:       shuffleNumber = 100;    break;
    }
}

void createPopupMenus() {
    SpeedMenu = glutCreateMenu(processSpeedMenu);
    glutAddMenuEntry("slow ",    slow);
    glutAddMenuEntry("medium ",  medium);
    glutAddMenuEntry("fast ",    fast);
    glutAddMenuEntry("express ", express);
    ShuffleMenu = glutCreateMenu(processShuffleMenu);
    glutAddMenuEntry("5 ",   five);
    glutAddMenuEntry("10 ",  ten);
    glutAddMenuEntry("15 ",  fifteen);
    glutAddMenuEntry("20 ",  twenty);
    glutAddMenuEntry("25 ",  twenty_five);
    glutAddMenuEntry("50 ",  fifty);
    glutAddMenuEntry("100 ", hundred);
    mainMenu = glutCreateMenu(processMainMenu);
    glutAddSubMenu("SpeedMenu",   SpeedMenu);
    glutAddSubMenu("ShuffleMenu", ShuffleMenu);
    // прикрепить меню к правой кнопке
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    //статус активности меню
    glutMenuStatusFunc(processMenuStatus);
}

// ------------------------------------ //
//             main()                   //
// -----------------------------------  //
int main(int argc, char **argv) {
    initv();
    LoadCube();
    loadCombins();
    which_stage();
    // инициализация Glut и создание окна
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
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
    return 1;
}

// функции работы с гранями
void saveG(int j, int i) {
    for (int k = 0; k < 3; k++) mg[k] = mcub[j].mCol[i][k];
}
void loadG(int j, int i) {
    for (int k = 0; k < 3; k++) mcub[j].mCol[i][k] = mg[k];
}
void setG(int j, int from, int to) {
    for (int k = 0; k < 3; k++) mcub[j].mCol[from][k] = mcub[j].mCol[to][k];
}
void rotG(int j, unsigned char key) {
//  {70, 1, 3, 0, 5}  // F 1 -> 3 -> 0 -> 5 -> 1
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
void setP(int j1, int j2) {
    for (int i = 0; i < kG; i++) {
        for (int k = 0; k < 3; k++) mcub[j1].mCol[i][k] = mcub[j2].mCol[i][k];
    }
}
void rotP(unsigned char key) {
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
void rotAll(int key0) {
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
void initv() {
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
void showG(int j, int i1, int i2, int i3, int i4, int k0) {
    glPushMatrix();
    // glRotatef(20.f, 1, 0, 0);
    // glRotatef(-25.f, 0, 1, 0);
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
    glPopMatrix();
}
void showCube() {
    for (int j = 0; j < kk; j++) // по кубиками  с центрами в mcub[ii][0-3]
        for (int i = 0; i < kG; i++)  // отрисовка граней: цвета mCol[0], mCol[1], ... 
            showG(j, mG[i][0], mG[i][1], mG[i][2], mG[i][3], i); // 
}

void showCubeG(unsigned char key) {
    for (int j = 0; j < kk; j++) { // по кубиками  с центрами в av[ii][0-3]
        if ((key == 'f' || key == 'g') && mcub[j].z !=  2 * a) continue; //  F G
        if ((key == 'u' || key == 'y') && mcub[j].y !=  2 * a) continue; //  U Y 
        if ((key == 'r' || key == 'e') && mcub[j].x !=  2 * a) continue; //  R E 
        if ((key == 'l' || key == 'k') && mcub[j].x != -2 * a) continue; //  L K 
        if ((key == 'd' || key == 's') && mcub[j].y != -2 * a) continue; //  D S 
        for (int i = 0; i < kG; i++)  // отрисовка поворачиваемой грани : с учетом лок.поворота
            showG(j, mG[i][0], mG[i][1], mG[i][2], mG[i][3], i); // 
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

void kcol(int j, int ng) {
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

bool contr() {
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

void shuffle() {
    for (int i = 0; i < shuffleNumber; i++) {
        QofKeys.push(keys[commands[random(6, commands.size() - 1)]]);
        do renderScene();
        while (k != 0);
    }
}

void loadCombins() {
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

str makeStrComm(int i) {
    str res = std::to_string(i) + " - ";
    for (pii p: combins[i - 1]) res += pairToKey[p];
    return res;
}

void doCombin(int i) {
    for (pii p: combins[i - 1]) {
        QofKeys.push(p);
        do renderScene();
        while (k != 0);
    }
}

void SaveCube() {
    std::ofstream file; file.open("save.txt");
    for (int i = 0; i < kk; i++)
        for (int j = 0; j < kG; j++) {
            vf vectorColor {mcub[i].mCol[j], mcub[i].mCol[j] + 3};
            file << mappingColors[vectorColor] << ' ';
        }
}

void LoadCube() {
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

// функции проверки выполнения стадии
bool white_cross_is_done() {
    bool res = false;

    vf cubics {7, 15, 17, 25};
    for (int i: cubics) {
        vf vectorColor {mcub[i].mCol[1], mcub[i].mCol[1] + 3};
        std::cout << i << " - " << stringColors[mappingColors[vectorColor]] << "\n";
    } std::cout << '\n';

    return res;
}
bool white_face_is_done() {
    bool res = false;

    vf cubics {6, 8, 16, 24, 26};
    for (int i: cubics) {
        vf vectorColor {mcub[i].mCol[1], mcub[i].mCol[1] + 3};
        std::cout << i << " - " << stringColors[mappingColors[vectorColor]] << "\n";
    } std::cout << '\n';
    
    return res;
}
bool second_layer_is_done() {
    bool res = false;
    return res;
}
bool last_layer_cross_is_done() {
    bool res = false;
    return res;
}
bool last_face_is_done() {
    bool res = false;
    return res;
}
bool set_edges_is_done() {
    bool res = false;
    return res;
}
bool set_corners_is_done() {
    bool res = false;
    return res;
}
