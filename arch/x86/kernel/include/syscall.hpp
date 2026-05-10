#pragma once
#include <stdint.h>
uint64_t syscall_dispatcher(uint64_t syscall_no, uint64_t arg1 = 0, uint64_t arg2 = 0);
extern "C" void syscall_handler_regs(uint64_t* rsp);