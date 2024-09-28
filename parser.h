#ifndef FOX_LEXER_H
#define FOX_LEXER_H

#include <Token.h>

int getInteger(const std::string& str);
// assembler functions
u_int16_t *assemble(std::vector<Token> *tokens);
int getNextLine(std::vector<Token> *token, int start);
int findLabel(std::vector<Token> *labels, std::string label);
void assembleLine(std::vector<Token> *tokens, std::vector<Token> *labels, int start, int end, u_int16_t *mem, int line);
void getInstruction(std::vector<Token> *tokens, std::vector<Token> *labels, int start, int end, u_int16_t *mem, int line);

#endif
