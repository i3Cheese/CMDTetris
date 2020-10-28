#include "PDCurses-3.8/curses.h"
#include "TetrisGame.h"
#include <map>
#include <random>
#include <time.h>
#include <iostream>

using namespace std;



int main(){
    // freopen("output.txt", "w", stdout);
    srand(time(NULL));

    // Иницализируем curses
    // cout << "START\n";
    initscr();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    cout << "COLORS\n";
    if (has_colors() == FALSE)
    {
        endwin();
        puts("\nYour terminal does not support color");
        return (1);
    }
    start_color(); //Активируем поддержку цвета
    use_default_colors();
    cout << "TG\n";
    TetrisGame tg(stdscr, 5, 10, 20, 12);
    cout << "LOOP\n";
    tg.game_loop();
    return 0;
}