#include "syscall.hpp"
#include "stdio.hpp"
#include "fat32.hpp"
#include "tsh.hpp"
#include "task.hpp"
#include "io.hpp"

// Вспомогательная функция, вызываемая из ассемблера
extern "C" void syscall_handler_regs(uint64_t* rsp) {
    // rsp[14] = rax (номер вызова), rsp[8] = rdi (arg1), rsp[9] = rsi (arg2)
    uint64_t syscall_no = rsp[14];
    uint64_t arg1 = rsp[8];
    uint64_t arg2 = rsp[9];
    uint64_t ret = syscall_dispatcher(syscall_no, arg1, arg2);
    rsp[14] = ret; // возвращаем значение в rax
}

uint64_t syscall_dispatcher(uint64_t syscall_no, uint64_t arg1, uint64_t arg2) {
    switch (syscall_no) {
        case 0: // print string
            term.print((const char*)arg1);
            break;
        case 1: { // read char (блокирующий)
            while (true) {
                char c = kbd_pop();
                if (c) return (uint64_t)c;
                Tasking::yield();
            }
            break;
        }
        case 2: // getcwd
            break;
        case 3: // open file
            break;
        case 4: // write file
            break;
        case 5: // create task
            break;
        case 6: // exit task
            Tasking::exit_current();
            break;
        default:
            break;
    }
    return 0;
}