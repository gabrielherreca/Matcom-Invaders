#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char playerName[50];
char *global_argv[2];

struct Enemy {
    int x;
    int y;
    int size;
    char *representation;

    int arriveTime;
    int life;
};

struct Enemy enemies[5]; // Array de enemigos

struct Shot {
    int x;
    int y;
    struct Shot *next;
    struct Shot *prev;
};

int ch, maxX, maxY, x, y; //(MaxX, MaxY) es el (ancho, alto) de la pantalla, (x,y) es la posicion actual de la nave
int SHIP_SIZE_X = 8;
int SHIP_SIZE_Y = 3;

int gameTime = 0;
bool isPlaying;
bool isAutopilot = false;
int autopilotMode = 0;
struct Shot *firstShot;
struct Shot *lastShot;

int score = 0;




void killEnemy(struct Enemy enemy){
    x = (enemy.x + enemy.size/2) - SHIP_SIZE_X/2;
    for (int j = 0; j < enemy.life; j++) {
        struct Shot *shot = malloc(sizeof(struct Shot));
        shot->x = x + SHIP_SIZE_X/2;
        shot->y = y - SHIP_SIZE_Y;
        shot->next = NULL;
        shot->prev = lastShot;
        lastShot->next = shot;
        lastShot = shot;
        usleep(300000);
    }
}

void welcomeScreen() {
    clear(); // Limpiar la pantalla
    mvprintw(LINES / 2, COLS / 2 - 15, "Bienvenido a Matcom Invaders"); // Imprimir el mensaje de bienvenida en el centro de la pantalla
    mvprintw(LINES / 2 + 1, COLS / 2 - 28, "Ingresa tu nombre por favor y presiona 'enter' para continuar"); // Imprimir las instrucciones debajo del mensaje de bienvenida
    refresh(); // Actualizar la pantalla
    echo(); // Habilitar el eco de los caracteres ingresados
    mvgetstr(LINES / 2 + 2, COLS / 2 - 10, playerName); // Solicitar el nombre del jugador
    noecho(); // Deshabilitar el eco de los caracteres ingresados


}


void FIFO(){
    struct Enemy oldest = enemies[0];
    for (int i = 1; i < 5; i++) {
        if (enemies[i].arriveTime < oldest.arriveTime)
            oldest = enemies[i];
    }
    killEnemy(oldest);
}

void SJF(){
    struct Enemy lifeless = enemies[0];
    for (int i = 1; i < 5; i++) {
        if (enemies[i].life < lifeless.life)
            lifeless = enemies[i];
    }
    killEnemy(lifeless);
}

void RR(int timeSlice){
    for (int i = 0; i < 5; i++) {
        x = (enemies[i].x + enemies[i].size/2) - SHIP_SIZE_X/2;
        for (int j = 0; j < timeSlice; j++) {
            struct Shot *shot = malloc(sizeof(struct Shot));
            shot->x = x + SHIP_SIZE_X/2;
            shot->y = y - SHIP_SIZE_Y;
            shot->next = NULL;
            shot->prev = lastShot;
            lastShot->next = shot;
            lastShot = shot;
            usleep(500000);
        }
    }
}


void* autopilot(){
    while(true){
        if (!isAutopilot) continue;
        switch (autopilotMode) {
            case 0:
                FIFO();
                break;
            case 1:
                SJF();
                break;
            case 2:
                RR(2);
                break;
        }
    }
}



void initEnemies() {
    srand(time(NULL)); // Inicializar la semilla del generador de números aleatorios
    for (int i = 0; i < 5; i++) {
        enemies[i].y = 0; // Posición y en el borde superior de la pantalla
        enemies[i].x = 4 + rand() % (maxX - 14); // Genera una posición x aleatoria dentro del rango de movimiento de la nave
        enemies[i].life = (rand() % 3)+1;
        enemies[i].size = 7;
    }

}

void gameOverScreen() {
    clear(); // Limpiar la pantalla

    int scoreCount = 0;

    FILE *file = fopen("scores.txt", "a+");
    if (file != NULL) {




        // Escribir el nombre del jugador en el archivo
        fprintf(file, "Player Name: %s\n", playerName);
        // Agregar la puntuación del jugador al final del archivo
        fprintf(file, "Score: %d\n", score);

        // Obtener la fecha y hora actual
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Formatear la fecha y hora en una cadena
        char date[20];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);

        // Escribir la fecha y hora en el archivo
        fprintf(file, "Date: %s\n", date);

        // Agregar el separador
        fprintf(file, "---------\n");

        fclose(file);
    }




    mvprintw(LINES / 2, COLS / 2 - 4, "GAME OVER"); // Imprimir "GAME OVER" en el centro de la pantalla
    mvprintw(LINES / 2 + 1, COLS / 2 - 6, "Your score: %d", score); // Imprimir los puntos máximos debajo de "GAME OVER"
    mvprintw(LINES / 2 + 6, COLS / 2 - 10, "Pulsa 'r' para jugar de nuevo o 'q' para salir"); // Imprimir las instrucciones para reiniciar el juego
    refresh(); // Actualizar la pantalla

    isPlaying = false; // Cambiar el estado del juego a falso

    // Esperar a que se presione una tecla
    char ch;
    do {
        ch = getch();
        if (ch == 'r') {
            // Reiniciar el programa
            execv(global_argv[0], global_argv);
        }
    } while (ch != 'q');

    endwin(); // Finalizar ncurses
    exit(0); // Salir del programa
}



void *moveEnemies(){
    while (isPlaying){
        sleep(2);
        for (int i = 0; i < 5; i++) {
            if (enemies[i].y < maxY) {
                enemies[i].y+=2 ;
            }else{
                gameOverScreen();
            }
        }
    }
    return NULL;

}

void printEnemies() {
    for (int i = 0; i < 5; i++) {
        if (enemies[i].x >= 0 && enemies[i].y >= 0) { // Solo imprime los enemigos que están dentro de la pantalla
            // Allocate memory for lifeStr
            char lifeStr[2];
            // Allocate memory for representation and construct it
            enemies[i].representation = malloc(8 * sizeof(char));
            sprintf(lifeStr, "%u", enemies[i].life);
            strcpy(enemies[i].representation, "|-(");
            strcat(enemies[i].representation, lifeStr);
            strcat(enemies[i].representation, ")-|");
            mvprintw(enemies[i].y, enemies[i].x, "%s", enemies[i].representation);
        }
    }
}
void printShip() {
////////IMPORTANTE/////LEER/////COMENTARIO/////DE/////ABAJO////////////////////////
    //Si cambias el diseño de la nave, hay que cambiar el SHIP_SIZE_X y SHIP_SIZE_Y (ancho - 1 y alto - 1 respectivamente)
    mvprintw(y - 3, x, "    ^");
    mvprintw(y-2, x, "  *****  ");
    mvprintw(y - 1 , x, "*********");
    mvprintw(y, x, "[]     []");
}


void moveAllShots(int cant) {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        shot->y -= cant;
        if (shot->y <= -1) {
            if (shot->prev == NULL && shot->next == NULL) { // Si es el único
                shot->x = -1;
            } else if (shot->prev == NULL) { // Si es el primero
                firstShot = shot->next;
                firstShot->prev = NULL;
            } else if (shot->next == NULL) { // Si es el último
                lastShot = shot->prev;
                lastShot->next = NULL;
            } else { // Si está en el medio
                shot->prev->next = shot->next;
                shot->next->prev = shot->prev;
            }
        }
        shot = shot->next;
    }
}
void* moveShoot(){
    while (true){
        usleep(300000); // 0.8 seconds
        moveAllShots(2);
    }
}

void printShots() {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        if (shot->x == -1) continue;
        mvprintw(shot->y, shot->x, "^");
        shot = shot->next;
    }
}


void checkCollisions() {
    struct Shot *shot = firstShot->next;
    while (shot != NULL) {
        struct Shot *nextShot = shot->next; // Guardar el siguiente disparo antes de modificar la lista
        for (int i = 0; i < 5; i++) {
            if (shot->x >= enemies[i].x && abs(enemies[i].x - shot->x) <= enemies[i].size
                    && abs(enemies[i].y - shot->y) <= 1) {

                enemies[i].life--;

                // Eliminar el disparo de la lista
                if (shot->prev == NULL && shot->next == NULL) { // Si es el único
                    shot->x = -1;
                } else if (shot->prev == NULL) { // Si es el primero
                    firstShot = shot->next;
                    firstShot->prev = NULL;
                } else if (shot->next == NULL) { // Si es el último
                    lastShot = shot->prev;
                    lastShot->next = NULL;
                } else { // Si está en el medio
                    shot->prev->next = shot->next;
                    shot->next->prev = shot->prev;
                }
                //free(shot); // Liberar la memoria del disparo

                //Eliminar la nave si la vida llega a 0
                if (enemies[i].life == 0){
                    enemies[i].y = 0; // Posición y en el borde superior de la pantalla
                    enemies[i].x = 4 + rand() % (maxX - 14); // Genera una posición x aleatoria dentro del rango de movimiento de la nave
                    enemies[i].life = (rand() % 3)+1;
                    enemies[i].size = 7;
                    enemies[i].arriveTime = gameTime;
                    // Aumentar la cuenta regresiva en 2 segundos
                    score += 1;
                }
                break; // Salir del bucle de enemigos ya que el disparo ha sido eliminado
            }
        }
        shot = nextShot; // Continuar con el siguiente disparo
    }
}
void showHUD(){
    char* autopilotModeStr;
    switch (autopilotMode) {
        case 0:
            autopilotModeStr = "FIFO";
            break;
        case 1:
            autopilotModeStr = "SJF";
            break;
        case 2:
            autopilotModeStr = "RR";
            break;
    }
    mvprintw(maxY-3, 3, "Score: %d", score);
    mvprintw(maxY-2, 3, "Autopilot: %s (%s) (Press 'f' | 's' | 'r')", isAutopilot? "ON": "OFF", autopilotModeStr);

}

void *refreshScreen() {
    while (isPlaying) {
        getmaxyx(stdscr, maxY, maxX); // Obtener las dimensiones máximas de la ventana
        clear();
        printShots();
        printShip();
        /*if (countdown >= 0) {
            attron(A_BOLD); // Poner texto en Negrita
            mvprintw(LINES / 2, COLS / 2, "%d", countdown);

        }*/
        showHUD();
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
                break;
            case 'a':
                isAutopilot = !isAutopilot;
                break;
            case 'f':
                autopilotMode = 0; //Elegir estrategia a seguir (FIFO)
                isAutopilot = true;
                break;
            case 's':
                autopilotMode = 1; //Elegir estrategia a seguir (SJF)
                isAutopilot = true;
                break;
            case 'r':
                autopilotMode = 2;//Elegir estrategia a seguir (RR)
                isAutopilot = true;
                break;
        }
    }
    return NULL;
}



int main(int argc, char *argv[]) {

    // Almacenar los argumentos de la línea de comandos
    global_argv[0] = argv[0];
    global_argv[1] = NULL;

    // Inicializar ncurses
    initscr();
    cbreak(); // Habilitar el modo cbreak para que getch() no espere el ingreso de una nueva línea

    // Mostrar la pantalla de bienvenida
    welcomeScreen();
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

    //Piloto Automatico
    pthread_t autopilot_thread_id;
    pthread_create(&autopilot_thread_id, NULL, autopilot, NULL);

    //Movimiento de las naves enemigas
    pthread_t moveEnemies_thread_id;
    pthread_create(&moveEnemies_thread_id, NULL, moveEnemies, NULL);

    // Finalizar ncurses
    pthread_join(printShip_thread_id, NULL);
    pthread_join(shoot_thread_id, NULL);
    pthread_join(refreshScreen_thread_id, NULL);
    pthread_join(autopilot_thread_id, NULL);
    pthread_join(moveEnemies_thread_id, NULL);
    endwin();
    free(firstShot);
    free(lastShot);

    return 0;
}