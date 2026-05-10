#pragma once

#include "iostream.hpp"
#include "string.hpp"
#include "fat32.hpp"
#include "stdio.hpp"

namespace Shell {

    const int HISTORY_SIZE = 10;
    char history[HISTORY_SIZE][64];
    int history_count = 0;

    void add_to_history(const char* cmd);

    void init();
    void run(); // Главный цикл ввода-вывода
    
    // Обработчики команд
    void cmd_ls(char* arg);
    void cmd_cd(char* arg);
    void cmd_mkdir(char* arg);
    void cmd_pwd(char* arg);
    void cmd_clear(char* arg);
    void cmd_help(char* arg);
    void cmd_touch(char* arg);
    void cmd_history(char* arg);
}
