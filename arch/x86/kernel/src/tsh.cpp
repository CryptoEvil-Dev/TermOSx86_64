#include "../include/tsh.hpp"

namespace Shell {

    char history[HISTORY_SIZE][64];
    int history_count = 0;


    typedef void (*command_func)(char*);

    struct Command {
        const char* name;
        command_func func;
        const char* help;
    };

    // Таблица соответствия команд функциям
    Command cmd_table[] = {
        {"ls",    cmd_ls,    "List directory content"},
        {"cd",    cmd_cd,    "Change directory"},
        {"mkdir", cmd_mkdir, "Create directory"},
        {"cat", cmd_cat, "Show content in file"},
        {"pwd",   cmd_pwd,   "Print working directory"},
        {"clear", cmd_clear, "Clear screen"},
        {"help",  cmd_help,  "Show this help"},
        {"touch", cmd_touch, "Create file"},
        {"history", cmd_history, "History"}
    };

    void run() {
        char input[128];
        while (true) {
            std::cout << "> ";
            std::cin >> input;

            if (input[0] == '!' && std::strlen(input) > 1) {
                int idx = input[1] - '1'; // Переводим символ в индекс (1 -> 0)
                if (idx >= 0 && idx < history_count) {
                    std::strcpy(input, history[idx]);
                    std::cout << input << std::endl; // Показываем, что выполняем
                } else {
                    std::cout << "No such command in history." << std::endl;
                    continue;
                }
            }
        
            add_to_history(input);


            // 1. Разделяем команду и аргумент
            int space_idx = std::find_char(input, ' ');
            char* cmd_name = input;
            char* arg = nullptr;

            if (space_idx != -1) {
                input[space_idx] = '\0';
                arg = &input[space_idx + 1];
            }

            // 2. Ищем команду в таблице
            bool found = false;
            for (uint64_t i = 0; i < sizeof(cmd_table)/sizeof(Command); i++) {
                if (std::strcmp(cmd_name, cmd_table[i].name) == 0) {
                    cmd_table[i].func(arg);
                    found = true;
                    break;
                }
            }

            if (!found && std::strlen(cmd_name) > 0) {
                std::cout << "Unknown command: " << cmd_name << std::endl;
            }
        }
    }

    // --- Реализации команд ---
    void cmd_ls(char* arg)    { Storage::fat32_ls(); }
    void cmd_pwd(char* arg)   { std::cout << "/" << std::endl; } // Пока заглушка
    void cmd_clear(char* arg) { term.clear(); }

    void add_to_history(const char* cmd) {
        if (std::strlen(cmd) == 0) return;

        // Сдвигаем историю, если она заполнена
        if (history_count == HISTORY_SIZE) {
            for (int i = 0; i < HISTORY_SIZE - 1; i++) {
                std::strcpy(history[i], history[i + 1]);
            }
            std::strcpy(history[HISTORY_SIZE - 1], cmd);
        } else {
            std::strcpy(history[history_count++], cmd);
        }
    }

    
    void cmd_cd(char* arg) {
        if (!arg) {
            std::cout << "Usage: cd <directory>" << std::endl;
            return;
        }
        Storage::fat32_cd(arg); // Должно вызывать функцию из fat32.cpp
    }

    void cmd_cat(char* arg) {
        if(!arg) {
            std::cout << "Usage: cat <filename>" << std::endl;
            return;
        }
        Storage::fat32_cat(arg);
    }

    void cmd_mkdir(char* arg) {
        if (!arg) { std::cout << "Usage: mkdir <name>" << std::endl; return; }
        Storage::fat32_mkdir(arg);
    }

    void cmd_help(char* arg) {
        std::cout << "TermOS Shell Commands:" << std::endl;
        for (uint64_t i = 0; i < sizeof(cmd_table)/sizeof(Command); i++) {
            std::cout << "  " << cmd_table[i].name << " - " << cmd_table[i].help << std::endl;
        }
    }

    void cmd_touch(char* arg) {
        if(!arg) {
            std::cout << "Usage: touch <filename>" << std::endl;
            return;
        }
        Storage::fat32_touch(arg);
    }

    void cmd_history(char* arg) {
        for (int i = 0; i < history_count; i++) {
            std::cout << i + 1 << ": " << history[i] << std::endl;
        }
    }
}
