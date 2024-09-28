#include <parser.h>
// Some macros because I'm lazy'
#define GET_REGISTER(arg) arg.value[0]-65
#define GET_NUMBER(arg) arg.type == INTEGER ? getInteger(arg.value) : arg.type == CHAR ? arg.value[0] : findLabel(labels, arg.value);
#define GET_ADDRESS(value) match(numberPattern, value) ? getInteger(value) : findLabel(labels, value);
#define IS_NUMBER_TOKEN(arg) arg.type == INTEGER || arg.type == IDENTIFIER || arg.type == CHAR

using namespace std;

static const string hexaPattern("^[0-9a-f]+h$");
static const string numberPattern("^[0-9a-f]+(b|h)?$");
static const string binaryPattern("^[0-1]+b$");

/** Nothing to see here. This function just put some more labels on the tokens and then pack them by line */
u_int16_t *assemble(vector<Token> *tokens) {
    // Now we pack the tokens by line and create a syntaxic tree
    int i = 0, j = 0, prev = 0, next = 0;
    vector<Token> *labels = new vector<Token>;
    vector<int> *lines = new vector<int>();
    lines->push_back(0);
    u_int16_t length = 0x100;
    string instructionSet("^(nop|ld|jp|jeq|jsup|jinf|add|sub|div|mul|or|and|xor|not|inc|syscall|hlt|mov|call|ret|pop|push|loop)$");
    while (i < tokens->size()) {
        j = getNextLine(tokens, i);
        for (int k = i; k < j; k++) {
            prev = k == i ? -1 : k-1;
            next = k == j-1 ? -1 : k+1;
            if (tokens->at(k).value == ":") { // LABELS
                if (prev == -1 or k != i+1) throw Exception{"Syntaxe error", "", &(tokens->at(i))};
                else if (tokens->at(prev).type == IDENTIFIER) { // We somehow remove the column
                    length--;
                    tokens->at(k).type = UNKNOW;
                    tokens->at(prev).type = LABEL;
                    tokens->at(k).line = length;
                    tokens->at(prev).line = length;    // We save the label's adress
                    labels->push_back(tokens->at(prev));
                } else {
                    throw Exception{"Syntaxe error", "unautorised label name", &(tokens->at(k))};
                }
            } else if (tokens->at(k).value == ".") { // COMMANDS
                if (next == -1) throw Exception{"Syntaxe error", "", &(tokens->at(i))};
                else if (tokens->at(next).type == IDENTIFIER and tokens->at(next).value == "fill") {
                    tokens->at(k).type = ASM_COMMAND;
                    tokens->at(next).type = UNKNOW;
                    next = k == j-2 ? -1 : k+2;
                    if (next == -1 or tokens->at(next).type != INTEGER) throw Exception{"Syntaxe error", "", &(tokens->at(i))};
                    tokens->at(k).line = length;
                    tokens->at(k).column = GET_NUMBER(tokens->at(next))
                    length+=tokens->at(k).column;
                    break;
                } else {
                    throw Exception{"Syntaxe error", "unautorised command name", &(tokens->at(k))};
                }
            } else if (match(instructionSet, tokens->at(k).value)) {
                if (k != i) {
                    if (tokens->at(prev).type != UNKNOW) // The column of a label
                        throw Exception{"Syntaxe error", "non-label token before an instruction", &(tokens->at(i))};
                }
                tokens->at(k).type = INSTRUCTION;
                tokens->at(k).line = length;
                if (match("^(ld|jp|jeq|jsup|jinf|call|loop)$", tokens->at(k).value))
                    length++;
                length++;
                break;
            } else if (tokens->at(k).type == IDENTIFIER) {
                if (match("^[A-H]$", tokens->at(k).value))
                    throw Exception{"Syntaxe error", "register as nothing do here", &(tokens->at(i))};
                tokens->at(k).line = length;
                length++;
            } else if (tokens->at(k).type == ADDRESS) {
                throw Exception{"Syntaxe error", "address as nothing do here", &(tokens->at(i))};
            } else if (tokens->at(k).type == STRING) {
                tokens->at(k).line = length;
                length += tokens->at(k).value.size()+1;
            } else {
                tokens->at(k).line = length;
                length++;
            }
        }
        lines->push_back(j);
        i = j;
    }
    // We will assemble each line while taking note of the spot where a label is used.
    u_int16_t *mem = (u_int16_t*)malloc(length*sizeof(u_int16_t));
    mem[0] = length;
    u_int16_t start;
    try {
        start = findLabel(labels, "start__");
    } catch (Exception e) {
        start = 0x100;
    }
    cout << start;
    mem[1] = start;
    Token temp;
    for (int j = 0; j < lines->size()-1; j++) {
        i = lines->at(j);/*
        for (int k = i; k < lines->at(j+1); k++) {
            mem[tokens->at(k).line] = tokens->at(k).value[0];
        }*/
        assembleLine(tokens, labels, lines->at(j), lines->at(j+1), mem, j+1);
    }
    return mem;
}

void assembleLine(vector<Token> *tokens, vector<Token> *labels, int start, int end, u_int16_t *mem, int line) {
    Token current = tokens->at(start);
    switch (current.type) {
        case LABEL: // We don't mind and begin the line from the next token
            if (start != end-2)
                assembleLine(tokens, labels, start+2, end, mem, line);
            break;
        case INSTRUCTION:
            getInstruction(tokens, labels, start, end, mem, line); break;
        default: // It can't be nothing than a stream of data
            int addr;
            for (int i = start; i < end; i++) {
                current = tokens->at(i);
                addr = current.line;
                switch (current.type) {
                    case IDENTIFIER: mem[addr] = findLabel(labels, current.value); break;
                    case INTEGER: mem[addr] = getInteger(current.value); break;
                    case ASM_COMMAND:
                        for (int j = 0; j < current.column; j++) {
                            mem[addr+j] = 0;
                        } break;
                    case STRING: int j;
                        for (j = 0; j < current.value.size(); j++) {
                            mem[addr+j] = current.value[j];
                        } mem[addr+j] = 0; break;
                    default: break;
                }
            } break;
    }
}

void getInstruction(vector<Token> *tokens, vector<Token> *labels, int start, int end, u_int16_t *mem, int line) {
    int addr = tokens->at(start).line;
    tokens->at(start).line = line;
    Exception argNumberException = Exception{"Syntaxe error", "invalid arguments number", &(tokens->at(start))};
    Exception argTypeException = Exception{"Syntaxe error", "invalid arguments type", &(tokens->at(start))};
    if (tokens->at(start).value == "ld") {
        if (end != start+3) throw argNumberException;
        switch (tokens->at(start+1).type) {
            case IDENTIFIER: {
                if (!match("^[A-H]$", tokens->at(start+1).value)) throw argTypeException;
                Token arg = tokens->at(start+2);
                if (IS_NUMBER_TOKEN(arg)) { // ld R <- const
                    mem[addr] = (0b01000000 + tokens->at(start+1).value[0]-65) << 8;
                    mem[addr+1] = GET_NUMBER(arg)
                } else if (arg.type == ADDRESS) { // ld R <- `addr`
                    mem[addr] = (0b01001000 + tokens->at(start+1).value[0]-65) << 8;
                    mem[addr+1] = GET_ADDRESS(arg.value)
                } else { throw argTypeException; }
            } break;
            case ADDRESS: {
                if (match("^[A-H]$", tokens->at(start+1).value)) {
                    Token arg = tokens->at(start+2);
                    if (IS_NUMBER_TOKEN(arg)) { // ld `R` <- const
                        mem[addr] = (0b01010000 + tokens->at(start+1).value[0]-65) << 8;
                        mem[addr+1] = GET_NUMBER(arg)
                    } else if (arg.type == ADDRESS) { // ld `R` <- `addr`
                        mem[addr] = (0b01011000 + tokens->at(start+1).value[0]-65) << 8;
                        mem[addr+1] = GET_ADDRESS(arg.value)
                    }
                } else {
                    Token arg = tokens->at(start+2);
                    if (!match("^[A-H]$", arg.value))
                        throw argTypeException;
                    if (arg.type == IDENTIFIER) { // ld `R` <- const
                        mem[addr] = (0b01100000 + arg.value[0]-65) << 8;
                        mem[addr+1] = GET_ADDRESS(tokens->at(start+1).value)
                    } else if (arg.type == ADDRESS) { // ld `R` <- `addr`
                        mem[addr] = (0b01101000 + arg.value[0]-65) << 8;
                        mem[addr+1] = GET_ADDRESS(tokens->at(start+1).value)
                    }
                }
            } break;
            default: throw argTypeException; break;
        }
    } else if (tokens->at(start).value == "mov") {
        if (end != start+3) throw argNumberException;
        // Both arguments should be registers
        Token arg1 = tokens->at(start+1);
        Token arg2 = tokens->at(start+2);
        if (!match("^[A-H]$", arg1.value) || !match("^[A-H]$", arg2.value))
            throw argTypeException;
        mem[addr] = (GET_REGISTER(arg1) << 8) + (GET_REGISTER(arg2) << 5);
        switch (arg1.type) {
            case IDENTIFIER: {
                if (arg2.type == IDENTIFIER) // mov r1 <- r2
                    mem[addr] |= 0b00001 << 11;
                else if (arg2.type == ADDRESS) // mov r1 <- `r2`
                    mem[addr] |= 0b00010 << 11;
            } break;
            case ADDRESS: {
                if (arg2.type == IDENTIFIER) // mov `r1` <- r2
                    mem[addr] |= 0b00011 << 11;
                else if (arg2.type == ADDRESS) // mov `r1` <- `r2`
                    mem[addr] |= 0b00100 << 11;
            } break;
            default: throw argTypeException; break;
        }
    } else if (match("^(jp|jeq|jsup|jinf|call|loop)$", tokens->at(start).value)) {
        if (end != start+2) throw argNumberException;
        Token arg1 = tokens->at(start+1);
        if (arg1.type != IDENTIFIER && arg1.type != ADDRESS)
            throw argTypeException;
        if (tokens->at(start).value == "jp")
            mem[addr] = 0b00110000;
        else if (tokens->at(start).value == "jeq")
            mem[addr] = 0b00110010;
        else if (tokens->at(start).value == "jsup")
            mem[addr] = 0b00110100;
        else if (tokens->at(start).value == "jinf")
            mem[addr] = 0b00110110;
        else if (tokens->at(start).value == "call")
            mem[addr] = 0b01110000;
        else if (tokens->at(start).value == "loop")
            mem[addr] = 0b00111000;
        if (arg1.type == IDENTIFIER) mem[addr] |= 0b0;
        else if (arg1.type == ADDRESS) mem[addr] |= 0b1;
        mem[addr] = mem[addr] << 8;
        mem[addr+1] = findLabel(labels, arg1.value);
    } else if (match("^add|sub|div|mul|or|and|xor$", tokens->at(start).value)) {
        if (end != start+4) throw argNumberException;
        // All arguments should be registers but the last two can be pointers
        Token arg1 = tokens->at(start+1);
        Token arg2 = tokens->at(start+2);
        Token arg3 = tokens->at(start+3);
        if (!match("^[A-H]$", arg1.value) || !match("^[A-H]$", arg2.value) || !match("^[A-H]$", arg3.value) || arg1.type != IDENTIFIER)
            throw argTypeException;
        mem[addr] = (GET_REGISTER(arg1) << 8) + (GET_REGISTER(arg2) << 5) + (GET_REGISTER(arg3) << 2);
        if (tokens->at(start).value == "add")
            mem[addr] |= 0b10100 << 11;
        else if (tokens->at(start).value == "sub")
            mem[addr] |= 0b10101 << 11;
        else if (tokens->at(start).value == "mul")
            mem[addr] |= 0b10110 << 11;
        else if (tokens->at(start).value == "div")
            mem[addr] |= 0b10111 << 11;
        else if (tokens->at(start).value == "and")
            mem[addr] |= 0b10000 << 11;
        else if (tokens->at(start).value == "or")
            mem[addr] |= 0b10001 << 11;
        else if (tokens->at(start).value == "xor")
            mem[addr] |= 0b10010 << 11;
        if (arg2.type == IDENTIFIER && arg3.type == IDENTIFIER) mem[addr] |= 0b00;
        else if (arg2.type == IDENTIFIER && arg3.type == ADDRESS) mem[addr] |= 0b01;
        else if (arg2.type == ADDRESS && arg3.type == IDENTIFIER) mem[addr] |= 0b10;
        else mem[addr] |= 0b11;
    } else if (tokens->at(start).value == "not") {
        if (end != start+3) throw argNumberException;
        Token arg1 = tokens->at(start+1);
        Token arg2 = tokens->at(start+2);
        if (!match("^[A-H]$", arg1.value) || !match("^[A-H]$", arg2.value)) throw argTypeException;
        mem[addr] = (0b10011000 << 8) + (GET_REGISTER(arg1) << 8) + (GET_REGISTER(arg2) << 5);
        if (arg1.type == IDENTIFIER && arg2.type == IDENTIFIER) mem[addr] |= 0b00;
        else if (arg1.type == IDENTIFIER && arg2.type == ADDRESS) mem[addr] |= 0b01;
        else if (arg1.type == ADDRESS && arg2.type == IDENTIFIER) mem[addr] |= 0b10;
        else mem[addr] |= 0b11;
    } else if (match("inc|push|pop", tokens->at(start).value)) { // not r1 <- !r2
        if (end != start+2) throw argNumberException;
        Token arg1 = tokens->at(start+1);
        if (!match("^[A-H]$", arg1.value)) throw argTypeException;
        if (tokens->at(start).value == "inc")
            mem[addr] = (0b00101000 | GET_REGISTER(arg1)) << 8;
        else if (tokens->at(start).value == "push")
            mem[addr] = (0b11001000 | GET_REGISTER(arg1)) << 8;
        else if (tokens->at(start).value == "pop")
            mem[addr] = (0b11010000 | GET_REGISTER(arg1)) << 8;
        if (arg1.type == IDENTIFIER ) mem[addr] |= 0b00;
        else if (arg1.type == ADDRESS) mem[addr] |= 0b01;
    } else if (tokens->at(start).value == "syscall") { // 0b11000000 00000000
        if (end != start+1) throw argNumberException;
        mem[addr] = 0b11000000 << 8;
    } else if (tokens->at(start).value == "hlt") { // 0b11111000 00000000
        if (end != start+1) throw argNumberException;
        mem[addr] = 0xff00;
    } else if (tokens->at(start).value == "ret") { // 0b11111000 00000000
        if (end != start+1) throw argNumberException;
        mem[addr] = 0b01111000 << 8;
    }
}

int getNextLine(vector<Token> *tokens, int start) {
    int end;
    int line;
    int prevLine = tokens->at(start).line;
    for (end = start; end < tokens->size(); end++) {
        line = tokens->at(end).line;
        if (prevLine != line) break;
        prevLine = line;
    }
    return end;
}

int charToInt(char c) {
    return (48 <= c && c <= 57) ? c-48 : c-87;
}

int getInteger(const string& str) {
    int number = 0;
    if (match(hexaPattern, str)) {
        for (int i = 0; i < str.size()-1; i++) {
            number = (number << 4) + charToInt(str[i]);
        }
    } else if (match(binaryPattern, str)) {
        for (int i = 0; i < str.size()-1; i++) {
            number = (number << 1) + str[i]-48;
        }
    } else { number = stoi(str); }
    return number;
}

int findLabel(vector<Token> *labels, string label) {
    for (int i = 0; i < labels->size(); i++) {
        if (labels->at(i).value == label) return labels->at(i).line-1;
    }
    throw Exception{"Syntax error", label+" inexisting label", nullptr};
}
