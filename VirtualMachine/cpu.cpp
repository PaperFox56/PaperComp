#include <cpu.h>

using namespace std;

void CPU::init() {
    csp = 0;
    R[E] = memSize; // stack pointer
    ip = mem[0]; // instruction pointer
    isRuning = true;
}

void CPU::interpreteNextOpcode() { // 0b00000 r1{000} r2{000} r3{000} 00
    u_int16_t opcode = mem[ip++];
    u_int8_t code = opcode >> 11;
    u_int8_t r1 = (opcode >> 8) & 0b111;
    u_int8_t r2 = (opcode >> 5) & 0b111;
    u_int8_t r3 = (opcode >> 2) & 0b111;
    //printf("%d %b\n", ip-1, opcode);
    switch (code) {
        // Load instructions
        case 0b00001: // mov r1 <- r2
            R[r1] = R[r2]; break;
        case 0b00010: // mov r1 <- `r2`
            R[r1] = mem[R[r2]]; break;
        case 0b00011: // mov `r1` <- r2
            mem[R[r1]] = R[r2]; break;
        case 0b00100: // mov `r1` <- `r2`
            mem[R[r1]] = mem[R[r2]]; break;
        case 0b01000: // ld r1 <- const
            R[r1] = mem[ip++]; break;
        case 0b01001: // ld r1 <- `addr`
            R[r1] = mem[mem[ip++]]; break;
        case 0b01010: // ld `r1` <- const
            mem[R[r1]] = mem[ip++]; break;
        case 0b01011: // ld `r1` <- `addr`
            mem[R[r1]] = mem[mem[ip++]];break;
        case 0b01100: // ld `addr` <- r1
            mem[mem[ip++]] = R[r1]; break;
        case 0b01101: // ld `addr` <- `r1`
            mem[mem[ip++]] = mem[R[r1]]; break;
        // Jump instructions
        case 0b00110: jump(r1); break;
        case 0b00111: { // loop addr
            if (R[C] != 0) {
                switch (r1 & 0b1) {
                    case 0: ip = mem[ip]; break;
                    case 1: ip = mem[mem[ip]]; break;
                }
                R[C]--;
            }
            break;
        }
        // Logical instructions
        case 0b10011: // not r1 <- !r2
            Not(opcode); break;
        case 0b10000: // and r1 <- r2 & r2
        case 0b10001: // or r1 <- r2 | r2
        case 0b10010: // xor r1 <- r2 ^ r2
        // Arithmetic instructions
        case 0b10100: // add r1 <- r2 + r2
        case 0b10101: // sub r1 <- r2 - r2
        case 0b10110: // mul r1 <- r2 * r2
        case 0b10111: // div r1 <- r2 / r2
            arithmetic(opcode); break;
        case 0b00101: { // inc r1; inc `r1`
            if ((opcode & 0b11) == 0b00)
                R[r1]++;
            else if ((opcode & 0b11) == 0b01)
                mem[R[r1]]++;
        } break;
        case 0b11001: { // push r1; push `r1`
            if ((opcode & 0b11) == 0b00)
                mem[--R[E]] = R[r1];
            else if ((opcode & 0b11) == 0b01)
                mem[--R[E]] = mem[R[r1]];
        } break;
        case 0b11010: { // pop r1; pop `r1`
            if ((opcode & 0b11) == 0b00)
                R[r1] = mem[R[E]++];
            else if ((opcode & 0b11) == 0b01)
                mem[R[r1]] = mem[R[E]++];
        } break;
        // Miscellanous
        case 0b01110: call(opcode); break;
        case 0b01111: ret(); break;
        case 0b11000: // syscall
            syscall(); break;
        case 0b11111: // hlt
            isRuning = false; break;
        default: break;
    }
}

void CPU::call(u_int8_t option) {
    u_int16_t target;
    switch (option & 0b1) {
        case 0: target = mem[ip++]; break;
        case 1: target = mem[mem[ip++]]; break;
    }
    callStack[csp++] = ip;
    for (int i = 0; i < 8; i++) {
        callStack[csp++] = R[i];
    }
    ip = target;
}

void CPU::ret() {
    for (int i = 7; i >= 0; i--) {
        R[i] = callStack[--csp];
    }
    ip = callStack[--csp];
}

void CPU::arithmetic(u_int16_t opcode) {
    u_int8_t code = opcode >> 11;
    u_int8_t r1 = (opcode >> 8) & 0b111;
    u_int8_t r2 = (opcode >> 5) & 0b111;
    u_int8_t r3 = (opcode >> 2) & 0b111;
    u_int8_t option = opcode & 0b11;

    u_int16_t op1, op2;
    switch(option) {
        case 0b00: op1 = R[r2]; op2 = R[r3]; break;
        case 0b01: op1 = R[r2]; op2 = mem[R[r3]]; break;
        case 0b10: op1 = mem[R[r2]]; op2 = R[r3]; break;
        case 0b11: op1 = mem[R[r2]]; op2 = mem[R[r3]]; break;
    }
    switch (code) {
        case 0b10000: { // and
            R[r1] = op1 & op2;
            break;
        }
        case 0b10001: { // or
            R[r1] = op1 | op2;
            break;
        }
        case 0b10010: { // xor
            R[r1] = op1 ^ op2;
            break;
        }
        case 0b10100: { // add
            R[r1] = op1 + op2;
            if (R[r1] < op1) carry = 1;
            else carry = 0;
            break;
        }
        case 0b10101: { // sub
            R[r1] = op1 - op2;
            if (op1 < op2) { carry = true; R[r1] = op2 - op1; }
            else carry = false;
            break;
        }
        case 0b10110: { // mul
            R[r1] = op1 * op2;
            int result = op1 * op2;
            if (R[r1] < result) {
                carry = true;
                R[F] = result >> 16;
            }
            else carry = false;
            break;
        }
        case 0b10111: { // div
            R[r1] = op1 / op2;
            int left = op1 % op2;
            if (left != 0) {
                carry = true;
                R[F] = left;
            }
            else carry = false;
            break;
        }
    }
    if (op1 == op2) equal = true;
    else equal = false;
}

void CPU::Not(u_int16_t opcode) {
    u_int8_t code = opcode >> 11;
    u_int8_t r1 = (opcode >> 8) & 0b111;
    u_int8_t r2 = (opcode >> 5) & 0b111;
    u_int8_t option = opcode & 0b11;

    switch(option) {
        case 0b00: R[r1] = -R[r2]-1; break;
        case 0b01: R[r1] = -mem[R[r2]]-1; break;
        case 0b10: mem[R[r1]] = -R[r2]-1; break;
        case 0b11: mem[R[r1]] = -mem[R[r2]]-1; break;
    }
}

int CPU::syscall() {
    switch (R[H]) {
        case 1: // print a character
            printf("%c", R[G]); break;
        case 2: { // print a string
            int c=1; int i = 0;
            while (c != 0) {
                c = mem[R[G] + i];
                printf("%c", c);
                i++;
            }
        } break;
        case 3: // print a number
            printf("%d", R[G]); break;
        case 4: // enter a number
            scanf("%hd", R+G); break;
        case 5: { // print a string
            string temp("");
            cin >> temp;
            for (int i=0; i < temp.size(); i++) {
                mem[R[G]+i] = temp[i];
            }
        } break;

    }
    return 0;
}

void CPU::jump(u_int8_t option) {
    u_int16_t target;
    switch (option & 0b1) {
        case 0: target = mem[ip++]; break;
        case 1: target = mem[mem[ip++]]; break;
    }
    switch (option >> 1) {
        case 0b00: ip = target; break;
        case 0b01:
            if (equal) ip = target; break;
        case 0b10:
            if (!equal and !carry) ip = target; break;
        case 0b11:
            if (!equal and carry) ip = target; break;
    }
}

CPU::CPU() {
}
