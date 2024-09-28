#ifndef TOKEN_H
#define TOKEN_H

#include <global.h>
#include <cstring>
#include <vector>
#include <regex>

//typedef enum TokenType TokenType;
enum TokenType { UNKNOW, LABEL, INSTRUCTION, ASM_COMMAND, PREPROCESSOR, IDENTIFIER, INTEGER, ADDRESS, CHAR, STRING, SEPARATOR };

typedef struct Token Token;
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

typedef struct Exception Exception;
struct Exception {
    std::string type;
    std::string message;
    Token *token;
    std::vector<Token> *children;
};

// Yeah, this structure only exist to be used by the tokenize function
// Edit: Finally, it's been more useful than I though'
typedef struct Position Position;
struct Position {
    int index = 0;
    int line = 0;
    int column = 0;
};

void printException(Exception e);

// Future me, please don't change anything below without serious reason because it could break everything
std::string getSurroundedBy(char ch, std::string_view src, Position *pos);
bool match(const std::string& pattern, const std::string& target);
bool isAlphanumerical(char c);
bool isDigit(char c);

void printToken(Token token, std::string_view end);
std::vector<Token> *tokenize(std::string_view src);
void classify(Token *token);
Token getNextToken(std::string_view src, Position *pos);

#endif
