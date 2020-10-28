#if !defined(__TetrisGame__)
#define __TetrisGame__

#include <vector>
#include <map>
#include <array>
#include <string>
#include "PDCurses-3.8/curses.h"
#include "PDCurses-3.8/panel.h"
#include <random>
using namespace std;

enum CP
{
    BACK_CP = 32,
    EMPTY_CP,
    RED_CP,
    CYAN_CP,
    YELLOW_CP,
    GREEN_CP,
    MAGENTA_CP,
    ORANGE_CP,
    BLUE_CP,
};
#define EMPTY_CELL (' ' | COLOR_PAIR(EMPTY_CP))
#define RED_CELL (' ' | COLOR_PAIR(RED_CP))
#define CYAN_CELL (' ' | COLOR_PAIR(CYAN_CP))
#define YELLOW_CELL (' ' | COLOR_PAIR(YELLOW_CP))
#define GREEN_CELL (' ' | COLOR_PAIR(GREEN_CP))
#define MAGENTA_CELL (' ' | COLOR_PAIR(MAGENTA_CP))
#define ORANGE_CELL (' ' | COLOR_PAIR(ORANGE_CP))
#define BLUE_CELL (' ' | COLOR_PAIR(BLUE_CP))
typedef array<array<chtype, 4>, 4> Tetramino;
enum TM_Status
{
    ABSENT,
    IN_AIR,
    ON_GROUND
};
Tetramino generate_tm(int n);
Tetramino rotate_left(Tetramino t);
Tetramino rotate_right(Tetramino t);

typedef vector<vector<chtype>> TFieldType;

struct TCoord
{
    int x, y;
    TCoord();
    TCoord(int y, int x);
    TCoord &operator+=(const TCoord &ot);
    bool operator==(const TCoord &ot) const;
    bool operator!=(const TCoord &ot) const;
    TCoord operator+(const TCoord &ot) const;
};

struct TetrisGame
{
private:
    enum Command
    {
        CMD_NOCOMMAND,
        CMD_EXIT,
        CMD_LEFT,
        CMD_RIGHT,
        CMD_DOWN,
        CMD_ROTATE_LEFT,
        CMD_ROTATE_RIGHT,
        CMD_PUT,
        CMD_PAUSE,
    };
    map<int, Command> cmd_table;

    chtype buffer_str[100];
    void print_string(string s, chtype attr = 0);
    int latency;
    int width, height; // Size of window
    int n, m;          // Size of field
    int st_left = 3, st_top = 3;
    int lines;         // Amount of deleted lines
    int lvl_lines;         // Amount of deleted lines on this level
    int points;        // Player's points
    int time;          // Time in game
    int level;
    int tics;    // Tics from last tm_status change
    int tm_tics; // Tics from creation tm
    bool in_game;
    TM_Status tm_status; // Status of tetramino
    TM_Status count_status();
    Tetramino tm;
    Tetramino tm_next;
    TCoord tm_coord; // Coords of tetramino
    TCoord tm_plus(int i, int j);
    bool check_coord(TCoord c);
    WINDOW *win; // Game window
    PANEL *panel;

    TFieldType field;

    enum GameStatus {IN_GAME, PAUSE, GAME_OVER};
    void  draw_all(GameStatus game_status = IN_GAME);
    void draw_field(GameStatus game_status = IN_GAME);
    Command get_command();
    void up_tm();   // up tm from walls and put tms
    void push_tm(); // push tm from walls
    bool down_tm();
    void rotate_tm_left();
    void rotate_tm_right();
    const int TO_DOWN_TICS = 32, TO_PUT_TICS = 64, ONE_TM_TICS = 1536;
    bool check_pos(TCoord c);
    bool in_left(TCoord c);
    bool in_right(TCoord c);
    void put_tm(); // add tetramino to field
    void new_tm();

    void delete_lines();
    void delete_line(int i);
    void count_line();
    vector<int> to_next_lvl = {0, 5, 10, 10, 10, 100};
public:
    TetrisGame(WINDOW *win, int y, int x, int n, int m, int lvl = 1, int latency = 1);
    void game_loop();
    void pause();
    void game_over();
};

#endif // __TetrisGame__
