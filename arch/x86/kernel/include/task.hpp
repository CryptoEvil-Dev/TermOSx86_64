#pragma once
#include <stdint.h>

namespace Tasking {
    enum TaskState { TASK_RUNNING, TASK_BLOCKED, TASK_ZOMBIE };

    struct Context {
        uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi;
        uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    } __attribute__((packed));

    struct Task {
        uint64_t pid;
        uint64_t* stack_top;
        uint64_t* stack_base;
        uint64_t stack_size;
        TaskState state;
        Task* next;
    };

    void init();
    Task* create_task(void (*entry)());
    void schedule();
    void yield();
    void block_current();
    void unblock(Task* task);
    void exit_current();
    Task* get_current();

    void set_foreground_task(Task* t);
    Task* get_foreground_task();
    void kill_task(Task* task);
}