#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

struct Enemy {
    int x;
    int y;
    char* representation;
};

struct Enemy enemies[5]; // Array de enemigos

struct Shot{
    int x;
    int y;
    struct Shot* next;
};
int ch, maxX, maxY, x, y;
bool isPlaying;
struct Shot* firstShot;
struct Shot* lastShot;

void initEnemies() {
    enemies[0].representation = "(*_*)";
    enemies[1].representation = "ƪ(@)ƪ";
    enemies[2].representation = "[-_-]";
    enemies[3].representation = "[¬º-°]¬";
    enemies[4].representation = "(°+°)";
}
void printShip(){
    mvprintw(y-1, x - 1, "   ^   ");
    mvprintw(y , x - 2, "  *****  ");
    mvprintw(y + 1, x - 2, "*********");
    mvprintw(y + 2, x - 3, " []     [] ");
}



void reduceAllShot(int cant){
    struct Shot* shot = firstShot;
    while (shot != NULL){
        shot->y -= cant;
        shot = shot->next;
    }
}
void* shoot(){
    while (true){
        usleep(300000); // 0.8 seconds
        reduceAllShot(2);
        if (lastShot->y == 0) break;
        struct Shot *shot = malloc(sizeof(struct Shot));
        shot->x = x+2;
        shot->y = y-2;
        shot->next = NULL;
        lastShot->next = shot;
        lastShot = shot;
    }
    return NULL;
}
void printShots(){
    struct Shot* shot = firstShot;
    while (shot != NULL){
        mvprintw(shot->y, shot->x, "^");
        shot = shot->next;
    }
}

void* refreshScreen(){
    while (isPlaying){
        clear();
        printShots();
        printShip();
        refresh();
        usleep(50000); //500000 = 0.5s
    }
}



void init(){
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
    firstShot->x = x;
    firstShot->y = y-2;
    firstShot->next = NULL;
    lastShot = firstShot;
    initEnemies();
}

void* moveShip(){
    while((ch = getch()) != 'q') {
        if (isPlaying==false) break;

        switch(ch) {
            case KEY_UP:
                y = (y > 0)? y - 1 : y;
                break;
            case KEY_DOWN:
                y = y < maxY - 3 ? y + 1 : y;
                break;
            case KEY_LEFT:
                x = x > 4 ? x - 2 : x; // Aumentar velocidad horizontal
                break;
            case KEY_RIGHT:
                x = x < maxX - 10 ? x + 2 : x; // Aumentar velocidad horizontal
                break;
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

    printShip();

    //Comenzar a disparar
    pthread_t shoot_thread_id;
    pthread_create(&shoot_thread_id, NULL, shoot, NULL);

    //Dibujar nave
    pthread_t printShip_thread_id;
    pthread_create(&printShip_thread_id, NULL, moveShip, NULL);



    // Finalizar ncurses
    pthread_join(printShip_thread_id, NULL);
    pthread_join(shoot_thread_id, NULL);
    pthread_join(refreshScreen_thread_id, NULL);
    endwin();
    free(lastShot);
    free(firstShot);
    return 0;

}