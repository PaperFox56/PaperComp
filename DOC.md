# PaperComp Virtual Machine Documentation

## Architecture Overview

### Memory Layout
- **Memory Size**: 0xFFFF (65535 words)
- **Word Size**: 16 bits (u_int16_t)
- **Call Stack Size**: 0xFF (255 entries)

### Registers
The CPU has 8 general-purpose registers (16-bit each):

| Register | Index | Purpose |
|----------|-------|---------|
| A | 0 | General Purpose |
| B | 1 | General Purpose |
| C | 2 | Loop Counter |
| D | 3 | General Purpose |
| E | 4 | Stack Pointer |
| F | 5 | Overflow Holder (for multiplication/division remainder) |
| G | 6 | Syscall Parameter |
| H | 7 | Syscall Number |

### Flags
- **Carry Flag**: Set when arithmetic overflow occurs or comparison condition is met
- **Equal Flag**: Set when two operands are equal (used by conditional jumps)

### Special Pointers
- **IP (Instruction Pointer)**: Points to the next instruction to execute
- **CSP (Call Stack Pointer)**: Manages the call stack for function calls

## Opcode Format

Each instruction is encoded as a 16-bit word with the following format:

```
[5-bit opcode][3-bit r1][3-bit r2][3-bit r3][2-bit option]
```

- **Opcode** (bits 15-11): Identifies the instruction type
- **r1** (bits 10-8): First register operand
- **r2** (bits 7-5): Second register operand
- **r3** (bits 4-2): Third register operand (if applicable)
- **Option** (bits 1-0): Instruction variant/addressing mode

## Instruction Set

### Load/Move Instructions (0b00001 - 0b01101)

| Opcode | Binary | Mnemonic | Description | Format |
|--------|--------|----------|-------------|--------|
| 1 | 0b00001 | MOV r1 ← r2 | Move register to register | mov r1, r2 |
| 2 | 0b00010 | MOV r1 ← `r2` | Load from memory address in r2 to r1 | mov r1, [r2] |
| 3 | 0b00011 | MOV `r1` ← r2 | Store r2 to memory address in r1 | mov [r1], r2 |
| 4 | 0b00100 | MOV `r1` ← `r2` | Load from memory at [r2] to memory at [r1] | mov [r1], [r2] |
| 8 | 0b01000 | LD r1 ← const | Load immediate constant into r1 | ld r1, const |
| 9 | 0b01001 | LD r1 ← `addr` | Load from address (following word) to r1 | ld r1, [addr] |
| 10 | 0b01010 | LD `r1` ← const | Store constant to memory at [r1] | ld [r1], const |
| 11 | 0b01011 | LD `r1` ← `addr` | Load from address to memory at [r1] | ld [r1], [addr] |
| 12 | 0b01100 | LD `addr` ← r1 | Store r1 to address (following word) | ld [addr], r1 |
| 13 | 0b01101 | LD `addr` ← `r1` | Store [r1] to address (following word) | ld [addr], [r1] |

### Jump Instructions (0b00110 - 0b00111)

| Opcode | Binary | Mnemonic | Description |
|--------|--------|----------|-------------|
| 6 | 0b00110 | JP | Jump to address (options: direct/indirect, conditional variants) |
| 7 | 0b00111 | LOOP addr | Loop: decrement C and jump if non-zero (option: direct/indirect addressing) |

#### Jump Variants (via option field):
- **Bits [1:0]**:
  - 0: Direct address from next word
  - 1: Indirect address from [next word]
- **Bits [3:2]**:
  - 00: JP (unconditional jump)
  - 01: JEQ (jump if equal)
  - 10: JSUP (jump if superior - not equal and not carry)
  - 11: JINF (jump if inferior - not equal and carry)

### Logical Instructions (0b10000 - 0b10011)

| Opcode | Binary | Mnemonic | Description |
|--------|--------|----------|-------------|
| 16 | 0b10000 | AND | AND operation: r1 ← r2 & r3 |
| 17 | 0b10001 | OR | OR operation: r1 ← r2 \| r3 |
| 18 | 0b10010 | XOR | XOR operation: r1 ← r2 ^ r3 |
| 19 | 0b10011 | NOT | NOT operation with addressing modes (4 variants) |

#### NOT Variants (via option field):
- 0b00: `r1 ← !r2` (bitwise NOT of register)
- 0b01: `r1 ← ![r2]` (bitwise NOT of memory contents)
- 0b10: `[r1] ← !r2` (store NOT of register to memory)
- 0b11: `[r1] ← ![r2]` (store NOT of memory to memory)

### Arithmetic Instructions (0b10100 - 0b10111)

| Opcode | Binary | Mnemonic | Description |
|--------|--------|----------|-------------|
| 20 | 0b10100 | ADD | Addition: r1 ← r2 + r3 |
| 21 | 0b10101 | SUB | Subtraction: r1 ← r2 - r3 |
| 22 | 0b10110 | MUL | Multiplication: r1 ← r2 * r3 |
| 23 | 0b10111 | DIV | Division: r1 ← r2 / r3 |

#### Arithmetic Flags:
- **ADD**: Sets carry if result overflows
- **SUB**: Sets carry if r2 < r3 (borrow), stores absolute difference in r1
- **MUL**: Sets carry if result > 16-bit max, stores overflow in register F
- **DIV**: Sets carry if remainder != 0, stores remainder in register F
- All operations set equal flag if r2 == r3

### Control Flow Instructions

| Mnemonic | Description | Assembled As |
|----------|-------------|--------------|
| CALL label | Call subroutine at label (pushes return address) | Uses opcode 0b10010 (internally) |
| RET | Return from subroutine | Uses opcode 0b10010 (internally) |
| HLT | Halt execution | Special instruction |
| NOP | No operation | Typically opcode 0 |

### Stack Instructions

| Mnemonic | Description |
|----------|-------------|
| PUSH r | Push register onto stack (SP = register E) |
| POP r | Pop from stack into register |
| INC r | Increment register |

## System Calls (Syscall)

The VM supports syscalls via the syscall instruction. The syscall number is passed in register H, and parameters in register G.

| Syscall # | Name | Description | Parameters |
|-----------|------|-------------|------------|
| 1 | Print Char | Print single character | G = ASCII code |
| 2 | Print String | Print null-terminated string | G = string address |
| 3 | Print Number | Print decimal number | G = value |
| 4 | Input Number | Read decimal number from input | G = target register address |
| 5 | Input String | Read string from input | G = string buffer address |

## Assembly Language Syntax

### Labels
```
labelName:
    instruction
```

### Instructions
```
mnemonic destination source [source2]
```

### Data Declarations
```
.fill count     ; Allocate count words of space
labelName: 0 0  ; Initialize data
```

### Comments
```
; This is a comment
```

### Literals
- **Decimal**: `123`
- **Hexadecimal**: `FFh` or `ff`
- **Binary**: `1010b`
- **Character**: `'A'` (single quote)
- **String**: `"Hello"` (double quote)
- **Address Reference**: `` `addr` `` (backtick)

## Example Programs

### Simple Loop
```assembly
start__: 
    ld C 10        ; Load counter
    ld A 0
loop_:
    inc A          ; Increment A
    loop loop_     ; Decrement C, jump if not zero
    hlt            ; Halt
```

### Function Call
```assembly
printValue:
    ld H 3         ; Syscall #3 (print number)
    mov G A
    syscall
    ret

start__:
    ld A 42
    call printValue
    hlt
```

## Memory Layout Example

```
0x0000: Entry point (instruction pointer initialized here)
0x0001-0x00FF: Used for program arguments
0x0100+: User program code
Stack grows downward from 0xFFFF
```

## Compiler Integration

The PaperComp Compiler converts assembly code to binary executable format:

1. **Tokenization**: Source code → tokens (via Token.cpp)
2. **Parsing**: Tokens → instruction stream (via parser.cpp)
3. **Assembly**: Instructions → binary opcodes
4. **Binary Format**: 
   - First word: number of words in program
   - Remaining words: instruction bytes

## Execution Model

The Virtual Machine executes instructions sequentially:

1. Load instruction from memory[IP]
2. Increment IP
3. Decode opcode and operands
4. Execute instruction
5. Update flags if applicable
6. Repeat until HLT or error