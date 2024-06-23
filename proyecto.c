#include <ncurses.h>

int main() {
    int ch, maxX, maxY, x, y;

    // Inicializar ncurses
    initscr();
    getmaxyx(stdscr, maxY, maxX); // Obtener las dimensiones máximas de la ventana
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    // Posición inicial de la nave en la parte inferior al medio
    x = maxX / 2;
    y = maxY - 3; // Ajustar según el tamaño de la nave

    // Dibujar la nave por primera vez
    mvprintw(y, x - 2, "/-\\");
    mvprintw(y + 1, x - 3, "<A>");
    mvprintw(y + 2, x - 2, "\\-/");

    while((ch = getch()) != 'q') {
        // Borrar la nave de la posición anterior
        for (int i = 0; i < 3; i++) {
            mvprintw(y + i, x - 3, "     ");
        }

        switch(ch) {
            case KEY_UP:
                y = y > 0 ? y - 1 : y;
                break;
            case KEY_DOWN:
                y = y < maxY - 3 ? y + 1 : y;
                break;
            case KEY_LEFT:
                x = x > 2 ? x - 2 : x; // Aumentar velocidad horizontal
                break;
            case KEY_RIGHT:
                x = x < maxX - 3 ? x + 2 : x; // Aumentar velocidad horizontal
                break;
        }

        // Dibujar la nave en la nueva posición
        mvprintw(y, x - 2, "/-\\");
        mvprintw(y + 1, x - 3, "<A>");
        mvprintw(y + 2, x - 2, "\\-/");
        refresh();
    }

    // Finalizar ncurses
    endwin();

    return 0;
}