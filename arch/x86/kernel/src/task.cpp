#include "task.hpp"
#include "idt.hpp"
#include "io.hpp"
#include "stdio.hpp"
#include "string.hpp"

namespace Tasking {
    static Task* current_task = nullptr;
    static Task* task_list = nullptr;
    static uint64_t next_pid = 1;

    static Task task_pool[16];
    static int task_pool_idx = 0;
    static uint64_t task_stacks[16][4096 / sizeof(uint64_t)];
    static int task_stack_idx = 0;

    static Task* foreground_task = nullptr;

    static Task* alloc_task() {
        if (task_pool_idx >= 16) return nullptr;
        Task* t = &task_pool[task_pool_idx++];
        t->stack_top = nullptr;
        t->stack_base = nullptr;
        t->stack_size = 0;
        t->state = TASK_RUNNING;
        t->next = nullptr;
        return t;
    }

    static uint64_t* alloc_stack() {
        if (task_stack_idx >= 16) return nullptr;
        return task_stacks[task_stack_idx++];
    }

    extern "C" void do_switch(uint64_t** old_stack_ptr, uint64_t* new_stack);

    extern "C" void schedule_wrapper() {
        Tasking::schedule();
    }

    void set_foreground_task(Task* t) { foreground_task = t; }
    Task* get_foreground_task() { return foreground_task; }

    void kill_task(Task* task) {
        if (task) task->state = TASK_ZOMBIE;
    }

    void init() {
        Task* kernel_task = alloc_task();
        if (!kernel_task) return;
        kernel_task->pid = next_pid++;
        kernel_task->stack_top = nullptr;
        kernel_task->stack_base = nullptr;
        kernel_task->stack_size = 0;
        kernel_task->state = TASK_RUNNING;
        kernel_task->next = kernel_task;
        current_task = kernel_task;
        task_list = kernel_task;
    }

    Task* create_task(void (*entry)()) {
        uint64_t* stack = alloc_stack();
        if (!stack) return nullptr;

        uint64_t* sp_initial = (uint64_t*)((uint8_t*)stack + 4096);
        uint64_t* sp = sp_initial;

        // Формируем iret frame (ss, rsp, rflags, cs, rip)
        *--sp = 0x10;                           // ss
        *--sp = (uint64_t)sp_initial;           // rsp – указывает на верх стека
        *--sp = 0x202;                          // rflags
        *--sp = 0x08;                           // cs
        *--sp = (uint64_t)entry;                // rip

        // Сохраняем регистры (порядок push: rax, rcx, rdx, rbx, rbp, rsi, rdi, r8..r15)
        *--sp = 0; // rax
        *--sp = 0; // rcx
        *--sp = 0; // rdx
        *--sp = 0; // rbx
        *--sp = 0; // rbp
        *--sp = 0; // rsi
        *--sp = 0; // rdi
        *--sp = 0; // r8
        *--sp = 0; // r9
        *--sp = 0; // r10
        *--sp = 0; // r11
        *--sp = 0; // r12
        *--sp = 0; // r13
        *--sp = 0; // r14
        *--sp = 0; // r15

        Task* task = alloc_task();
        if (!task) return nullptr;

        task->pid = next_pid++;
        task->stack_top = sp;   // указывает на r15
        task->stack_base = stack;
        task->stack_size = 4096;
        task->state = TASK_RUNNING;

        task->next = task_list->next;
        task_list->next = task;

        return task;
    }

    void schedule() {
        if (!current_task) return;

        Task* next = current_task->next;
        while (next->state != TASK_RUNNING) {
            next = next->next;
            if (next == current_task) return;
        }
        if (next == current_task) return;

        Task* prev = current_task;
        current_task = next;
        do_switch(&prev->stack_top, next->stack_top);
    }

    void yield() {
        asm volatile("int $0x81");
    }

    void block_current() {
        current_task->state = TASK_BLOCKED;
        schedule();
    }

    void unblock(Task* task) {
        task->state = TASK_RUNNING;
    }

    void exit_current() {
        current_task->state = TASK_ZOMBIE;
        schedule();
    }

    Task* get_current() {
        return current_task;
    }
}