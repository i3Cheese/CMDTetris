#include "TetrisGame.h"
#include "PDCurses-3.8/curses.h"
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

TCoord::TCoord() : x(0), y(0){}
TCoord::TCoord(int y, int x) : y(y), x(x){}
TCoord& TCoord::operator+=(const TCoord &ot){
    x += ot.x;
    y += ot.y;
    return *this;
}
TCoord TCoord::operator+(const TCoord &ot) const {
    return TCoord(y + ot.y, x + ot.x);
}
bool TCoord::operator==(const TCoord &ot) const {
    return y == ot.y && x == ot.x;
}
bool TCoord::operator!=(const TCoord &ot) const {
    return !(*this == ot);
}


void TetrisGame::print_string(string s, chtype attr){
    for (int i = 0; i < s.size(); ++i){
        buffer_str[i] = s[i] | attr;
    }
    buffer_str[s.size()] = '\0';
    waddchstr(win, buffer_str);
}


Tetramino generate_tm(int n){
    Tetramino res;
    Tetramino::value_type one_line;
    one_line.fill(EMPTY_CELL);
    res.fill(one_line);
    switch(n){
    case 0:
        one_line.fill(CYAN_CELL);
        res[1] = one_line;
        break;
    case 1:
        res[1][1] = res[1][2] = res[2][1] = res[2][2] = YELLOW_CELL;
        break;
    case 2:
        res[2][0] = res[2][1] = res[1][1] = res[1][2] = GREEN_CELL;
        break;
    case 3:
        res[1][0] = res[1][1] = res[2][1] = res[2][2] = RED_CELL;
        break;
    case 4:
        res[2][0] = res[2][1] = res[2][2] = res[1][1] = MAGENTA_CELL;
        break;
    case 5:
        res[2][0] = res[2][1] = res[2][2] = res[1][2] = ORANGE_CELL;
        break;
    case 6:
        res[1][0] = res[2][0] = res[2][1] = res[2][2] = BLUE_CELL;
        break;
    case 7:
        one_line.fill(YELLOW_CELL);
        res[0] = one_line;
        res[0][3] = EMPTY_CELL;
        res[1][1] = YELLOW_CELL;
        res[2][1] = YELLOW_CELL;
        res[3][1] = MAGENTA_CELL;
        break;
    }
    return res;
}

Tetramino rotate_right(Tetramino t){
    Tetramino ans;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            ans[i][j] = t[j][3 - i];
        }
    }
    return ans;
}

Tetramino rotate_left(Tetramino t){
    Tetramino ans;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            ans[j][3 - i] = t[i][j];
        }
    }
    return ans;
}


TetrisGame::TetrisGame(WINDOW *_win, int y, int x, int _n, int _m, int lvl, int _latency)
    : width(2 * _m), height(_n), n(_n), m(_m), level(lvl), latency(_latency){
    win = _win;
    keypad(win, true);

    cout << "INIT\n";
    init_pair(BACK_CP, COLOR_WHITE, COLOR_BLACK);
    init_pair(EMPTY_CP, COLOR_GRAYTEXT, COLOR_GRAYTEXT);
    init_pair(RED_CP, COLOR_RED, COLOR_RED);
    init_pair(CYAN_CP, COLOR_CYAN, COLOR_CYAN);
    init_pair(YELLOW_CP, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(GREEN_CP, COLOR_GREEN, COLOR_GREEN);
    init_pair(MAGENTA_CP, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(ORANGE_CP, 12, 12);
    init_pair(BLUE_CP, COLOR_HOTLIGHT , COLOR_HOTLIGHT);

    tm_status = ABSENT;
    tics = 0;
    lines = 0;
    points = 0;
    cout << "field\n";
    field.assign(n, vector<chtype>(m, EMPTY_CELL));

    cmd_table[27] = CMD_EXIT; // esc key
    cmd_table[KEY_LEFT] = CMD_LEFT;
    cmd_table[KEY_RIGHT] = CMD_RIGHT;
    cmd_table[KEY_DOWN] = CMD_DOWN;
    cmd_table[KEY_UP] = CMD_ROTATE_LEFT;
    cmd_table['z'] = CMD_ROTATE_RIGHT;
    cmd_table[' '] = CMD_PUT;
    cmd_table['p'] = CMD_PAUSE;
}

bool TetrisGame::check_coord(TCoord c){
    return (0 <= c.y && c.y < n) && (0 <= c.x && c.x < m);
}

TM_Status TetrisGame::count_status(){
    if (tm_status == ABSENT)
        return ABSENT;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            if (tm[i][j] == EMPTY_CELL)
                continue;
            TCoord down(tm_coord.y + i + 1, tm_coord.x + j);
            if (down.y >= n || (check_coord(down) && field[down.y][down.x] != EMPTY_CELL))
                return ON_GROUND;
        }
    }
    return IN_AIR;
}

void TetrisGame::draw_field(GameStatus game_status){
    for (int i = 0; i < n; ++i){
        wmove(win, st_top + i, st_left);
        for (int j = 0; j < m; ++j){
            if (game_status == PAUSE){
                waddch(win, EMPTY_CELL);
                waddch(win, EMPTY_CELL);
            } else {
                waddch(win, field[i][j]);
                waddch(win, field[i][j]);
            }
        }
    }
    if (tm_status != ABSENT && game_status != PAUSE){
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                if (tm[i][j] == EMPTY_CELL)
                    continue;
                TCoord c = tm_plus(i, j);
                if (!check_coord(c)){
                    continue;
                }
                wmove(win, c.y +  st_top,  st_left + 2 * c.x);
                waddch(win, tm[i][j]);
                waddch(win, tm[i][j]);
            }
        }
    }
    if (game_status == PAUSE){
        string msg = "+ PAUSE +";
        wmove(win, st_top + height / 2, st_left + width / 2 - msg.size() / 2);
        print_string(msg);
    }
}

void TetrisGame::draw_all(GameStatus game_status){
    draw_field(game_status);
    string title;
    if (game_status == GAME_OVER){
        title = "=== GAME OVER ===";
    } else {
        title = "=== Tetris ===";
    }
    wmove(win, 1, st_left + (width - title.size()) / 2);
    print_string(title);
    wmove(win, st_top + height, st_left);
    print_string("Level: " + to_string(level));
    string lines_str = "Lines: " + to_string(lines);
    wmove(win, st_top + height, st_left + width - lines_str.size());
    print_string(lines_str);

    if (tm_status != ABSENT){
        // draw tm_next
        wmove(win, st_top, st_left + width + 2);
        print_string("Next:");
        for (int i = 0; i < 4; ++i){
            wmove(win, st_top + i + 1, st_left + width + 2);
            for (int j = 0; j < 4; ++j){
                waddch(win, ' ');
                waddch(win, ' ');
            }
        }
        for (int i = 0; i < 4; ++i){
            for (int j = 0; j < 4; ++j){
                if (tm_next[i][j] == EMPTY_CELL) continue;
                wmove(win, st_top + i + 1,  st_left + width + 2 + 2*j);
                waddch(win, tm_next[i][j]);
                waddch(win, tm_next[i][j]);
            }
        }
    }
}

TCoord TetrisGame::tm_plus(int i, int j){
    return tm_coord + TCoord(i, j);
}

void TetrisGame::new_tm(){
    tm = tm_next;
    int num = rand() % 7;
    if (level >= to_next_lvl.size()) num = 7;
    tm_next = generate_tm(rand() % 7);
    tm_coord = TCoord(0, m / 2 - 2);
    up_tm();
    tm_status = IN_AIR;
    tics = 0;
    tm_tics = 0;
}

bool TetrisGame::down_tm(){
    TCoord nc = tm_plus(1, 0);
    if (check_pos(nc)){
        tm_coord = nc;
        return true;
    }
    return false;
}

void TetrisGame::rotate_tm_left(){
    tm = rotate_left(tm);
    push_tm();
}

void TetrisGame::rotate_tm_right(){
    tm = rotate_right(tm);
    push_tm();
}

void TetrisGame::up_tm(){
    while(!check_pos(tm_coord)){
        tm_coord += TCoord(-1, 0);
    }
}

void TetrisGame::push_tm(){
    while(in_left(tm_coord)){
        tm_coord += TCoord(0, 1);
    }
    while(in_right(tm_coord)){
        tm_coord += TCoord(0, -1);
    }
    up_tm();
}

void TetrisGame::put_tm(){
    tm_status = ABSENT;
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            if (tm[i][j] == EMPTY_CELL)
                continue;
            TCoord n_c = tm_coord + TCoord(i, j);
            if (check_coord(n_c))
                field[n_c.y][n_c.x] = tm[i][j];
            else if (n_c.y < 0){
                in_game = false;
                return;
            }
        }
    }
    new_tm();
}

bool TetrisGame::check_pos(TCoord c){
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            if (tm[i][j] == EMPTY_CELL)
                continue;
            auto cell = c + TCoord(i, j);
            if (check_coord(cell) && field[cell.y][cell.x] != EMPTY_CELL)
                return false;
            else if (cell.x < 0 || m <= cell.x || cell.y >= n)
                return false;
        }
    }
    return true;
}

bool TetrisGame::in_left(TCoord c){
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            if (tm[i][j] == EMPTY_CELL)
                continue;
            auto cell = c + TCoord(i, j);
            if (cell.x < 0)
                return true;
        }
    }
    return false;
}

bool TetrisGame::in_right(TCoord c){
    for (int i = 0; i < 4; ++i){
        for (int j = 0; j < 4; ++j){
            if (tm[i][j] == EMPTY_CELL)
                continue;
            auto cell = c + TCoord(i, j);
            if (cell.x >= m)
                return true;
        }
    }
    return false;
}

TetrisGame::Command TetrisGame::get_command(){
    int key = wgetch(win);
    flushinp();
    if (cmd_table.count(key)){
        return cmd_table[key];
    } else {
        return CMD_NOCOMMAND;
    }
}

void TetrisGame::count_line(){
    ++lines;
    ++lvl_lines;
    if (to_next_lvl.size() > level && to_next_lvl[level] <= lvl_lines){
        lvl_lines = 0;
        ++level;
    }
}

void TetrisGame::delete_line(int num){
    count_line();
    for (int i = num; i > 0; --i)
        for (int j = 0; j < m; ++j)
            field[i][j] = field[i - 1][j];
    for (int j = 0; j < m; ++j)
        field[0][j] = EMPTY_CELL;
}

void TetrisGame::delete_lines(){
    for (int i = 0; i < n; ++i){
        bool to_del = true;
        for (int j = 0; j < m; ++j)
            if (field[i][j] == EMPTY_CELL){
                to_del = false;
                break;
            }
        if (to_del)
            delete_line(i);
    }
}

void TetrisGame::pause(){
    while (true){
        switch(get_command()){
        case CMD_EXIT:
            in_game = false;
            return;
        case CMD_NOCOMMAND:
            break;
        default:
            return;
        }
        draw_all(PAUSE);
        Sleep(latency);
    }
}

void TetrisGame::game_over(){
    draw_all(GAME_OVER);
    Sleep(75);
    while (true){
        switch(get_command()){
        case CMD_NOCOMMAND:
            break;
        default:
            return;
        }
        draw_all(GAME_OVER);
        Sleep(latency);
    }
}

void TetrisGame::game_loop(){
    Command com = CMD_NOCOMMAND;
    tm_next = generate_tm(rand() % 7);
    new_tm();
    in_game = true;
    lines = 0;
    lvl_lines = 0;
    points = 0;
    TCoord new_coord;
    while (in_game){
        bool in_move = false;
        com = get_command();
        if (com == CMD_EXIT){
            in_game = false;
            continue;
        }
        if (com == CMD_PAUSE){
            pause();
        }
        if (tm_status != ABSENT){
            switch (com){
            case CMD_LEFT:
                new_coord = tm_coord + TCoord(0, -1);
                if (tm_coord != new_coord && check_pos(new_coord)){
                    tm_coord = new_coord;
                    in_move = true;
                }
                break;
            case CMD_RIGHT:
                new_coord = tm_coord + TCoord(0, 1);
                if (tm_coord != new_coord && check_pos(new_coord)){
                    tm_coord = new_coord;
                    in_move = true;
                }
                break;
            case CMD_ROTATE_LEFT:
                in_move = true;
                rotate_tm_left();
                break;
            case CMD_ROTATE_RIGHT:
                in_move = true;
                rotate_tm_right();
                break;
            case CMD_DOWN:
                if(down_tm()){
                    tics = 0;
                }
                break;
            case CMD_PUT:
                while(down_tm());
                put_tm();
                break;
            }
        }
        TM_Status new_st = count_status();
        if (new_st != tm_status){
            tics = 0;
            tm_status = new_st;
        }
        if (tm_status == IN_AIR && ((TO_DOWN_TICS >> (level - 1)) <= tics)){
            down_tm();
            tics = 0;
        }
        if (tm_status == ON_GROUND){
            if (in_move)
                tics = 0;
            if (TO_PUT_TICS <= tics || tm_tics >= ONE_TM_TICS){
                put_tm();
            }
        }
        delete_lines();
        ++tics;
        ++tm_tics;
        wclear(win);
        draw_all();
        Sleep(latency);
    }
    game_over();
}
