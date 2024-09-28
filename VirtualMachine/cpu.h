#ifndef CPU_H
#define CPU_H

#include <global.h>

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4 // Stack pointer
#define F 5 // Overflow holder
#define G 6 // syscall's parameter
#define H 7 // syscall number

#define memSize 0xffff
#define callStackSize 0xff

typedef unsigned short u_int16_t;
typedef unsigned char u_int8_t;


class CPU {
public:
    CPU();
    void interpreteNextOpcode();

    void init();
    bool isRuning;
    u_int16_t mem[memSize] = {0};

private:
    int syscall();
    void ret();
    void call(u_int8_t opcode);
    void jump(u_int8_t option);
    void Not(u_int16_t opcode);
    void arithmetic(u_int16_t opcode);
    u_int16_t R[8] = {0};
    u_int16_t callStack[callStackSize] = {0}; //
    u_int16_t ip; // instruction pointer
    u_int8_t csp; // call stack pointer
    bool carry, equal;
};

#endif
