cmake_minimum_required(VERSION 3.10)


project(tetris)

set(SOURCE main.cpp TetrisGame.cpp)

add_executable(tetris ${SOURCE})

target_link_libraries(tetris pdcurses)
