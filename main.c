#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct Enemy {
    int x;
    int y;
    int size;
    char *representation;
};

struct Enemy enemies[5]; // Array de enemigos

struct Shot {
    int x;
    int y;
    struct Shot *next;
    struct Shot *prev;
};
int ch, maxX, maxY, x, y;
int SHIP_SIZE_X = 8;
int SHIP_SIZE_Y = 3;


bool isPlaying;
struct Shot *firstShot;
struct Shot *lastShot;

void initEnemies() {
    srand(time(NULL)); // Inicializar la semilla del generador de números aleatorios
    for (int i = 0; i < 5; i++) {
        enemies[i].y = 0; // Posición y en el borde superior de la pantalla
        enemies[i].x = 4 + rand() % (maxX - 14); // Genera una posición x aleatoria dentro del rango de movimiento de la nave
    }
    enemies[0].representation = "(*_*)";
    enemies[1].representation = "ƪ(@)ƪ";
    enemies[2].representation = "[-_-]";
    enemies[3].representation = "[¬º-°]¬";
    enemies[4].representation = "(°+°)";
    for (int i = 0; i < 5; ++i) {
        enemies[i].size = strlen(enemies[i].representation);
    }
}

void printEnemies() {
    for (int i = 0; i < 5; i++) {
        if (enemies[i].x >= 0 && enemies[i].y >= 0) { // Solo imprime los enemigos que están dentro de la pantalla
            mvprintw(enemies[i].y, enemies[i].x, "%s", enemies[i].representation);
        }
    }
}
void printShip() {
////////IMPORTANTE/////LEER/////COMENTARIO/////DE/////ABAJO////////////////////////
    //Si cambias el diseño de la nave, hay que cambiar el SHIP_SIZE_X y SHIP_SIZE_Y (ancho/alto - 1)
    mvprintw(y - 3, x, "    ^");
    mvprintw(y-2, x, "  *****  ");
    mvprintw(y - 1 , x, "*********");
    mvprintw(y, x, "[]     []");
}


void reduceAllShot(int cant) {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        shot->y -= cant;
        if (shot->y <= 0) {
            if (shot->prev == NULL && shot->next == NULL)
                shot->x = -1;
            else if (shot->prev == NULL) //Si es el primero
                firstShot = shot->next;
            else if (shot->next == NULL) //Si es el ultimo
                lastShot = shot->prev;
            else
                shot->prev->next = shot->next;
        }
        shot = shot->next;
    }
}
void* moveShoot(){
    while (true){
        usleep(300000); // 0.8 seconds
        reduceAllShot(2);
    }
}

void printShots() {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        if (shot->x == -1) break;
        mvprintw(shot->y, shot->x, "^");
        shot = shot->next;
    }
}


void checkCollisions() {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        for (int i = 0; i < 5; i++) {
            if (shot->x >= enemies[i].x && abs(enemies[i].x - shot->x) <= enemies[i].size && abs(enemies[i].y - shot->y) <= 1) {
                // El enemigo ha sido golpeado, lo hacemos reaparecer en la parte superior de la pantalla
                enemies[i].y = 0; // Posición y en el borde superior de la pantalla
                enemies[i].x = 4 + rand() % (maxX - 14); // Genera una posición x aleatoria dentro del rango de movimiento de la nave
            }
        }
        shot = shot->next;
    }
}

void *refreshScreen() {
    while (isPlaying) {
        getmaxyx(stdscr, maxY, maxX); // Obtener las dimensiones máximas de la ventana
        clear();
        printShots();
        printShip();
        printEnemies();
        checkCollisions();
        refresh();

        usleep(50000); //500000 = 0.5s
    }
}


void init() {
    // Inicializar ncurses
    initscr();
    getmaxyx(stdscr, maxY, maxX); // Obtener las dimensiones máximas de la ventana
    //raw(); //bloquea las funciones del teclado como ctrl-C
    keypad(stdscr, TRUE); //habilita la lectura de las teclas de función y de las teclas de flecha del teclado.
    noecho(); //los caracteres que el usuario ingresa no se mostrarán en la pantalla
    curs_set(0); //El parametro 0 hace que el cursor sea invisible.
    // Posición inicial de la nave en la parte inferior al medio
    x = maxX / 2;
    y = maxY - 3; // Ajustar según el tamaño de la nave
    isPlaying = true;
    firstShot = malloc(sizeof(struct Shot));
    firstShot->x = -1;
    firstShot->y = y - 2;
    firstShot->next = NULL;
    firstShot->prev = NULL;
    lastShot = firstShot;
    initEnemies();
}

void *moveShip() {
    while ((ch = getch()) != 'q') {
        if (isPlaying == false) break;

        switch (ch) {
            case KEY_UP:
                y = (y - SHIP_SIZE_Y-1 > 0) ? y - 1 : y;
                break;
            case KEY_DOWN:
                y = y+1 < maxY ? y + 1 : y;
                break;
            case KEY_LEFT:
                x = x > 2 ? x - 2 : x; // Aumentar velocidad horizontal
                break;
            case KEY_RIGHT:
                x = x < maxX - SHIP_SIZE_X-2 ? x + 2 : x; // Aumentar velocidad horizontal
                break;
            case 32:
                struct Shot *shot = malloc(sizeof(struct Shot));
                shot->x = x + SHIP_SIZE_X/2;
                shot->y = y - SHIP_SIZE_Y;
                shot->next = NULL;
                shot->prev = lastShot;
                lastShot->next = shot;
                lastShot = shot;
        }
    }
    return NULL;
}


int main() {

    //Inicializar ncurses y variables
    init();

    //refrescar
    pthread_t refreshScreen_thread_id;
    pthread_create(&refreshScreen_thread_id, NULL, refreshScreen, NULL);

    //Comenzar a disparar
    pthread_t shoot_thread_id;
    pthread_create(&shoot_thread_id, NULL, moveShoot, NULL);

    //Dibujar nave
    pthread_t printShip_thread_id;
    pthread_create(&printShip_thread_id, NULL, moveShip, NULL);

    // Finalizar ncurses
    pthread_join(printShip_thread_id, NULL);
    pthread_join(shoot_thread_id, NULL);
    pthread_join(refreshScreen_thread_id, NULL);
    endwin();
    free(lastShot);

    return 0;
}