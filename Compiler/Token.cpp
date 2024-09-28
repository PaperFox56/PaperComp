#include <Token.h>

using namespace std;

/**
 * This file contains all functions needed to tokenize a source code
*/


/** Create a list of tokens from the source code */
vector<Token> *tokenize(string_view src) {
    vector<Token> *tokens = new vector<Token>();
    Position pos{0, 1, 1};
    Token token;
    while (pos.index < src.size()) {
        token = getNextToken(src, &pos);
        if (token.value != "") {
            tokens->push_back(token);
            if (token.type == CHAR && token.value.size() > 1)
                throw Exception{"Syntaxe error", "to much characters",&(tokens->back())};
            if (tokens->back().type != UNKNOW)
                continue;
            else if (match("^[0-9]+(b|h)?$", token.value))
                tokens->back().type = INTEGER;
            else if (match("^[0-9a-f]+b|h$", token.value)) {
                tokens->back().type = INTEGER;
                if (match("[a-f]", token.value) && !match("^[0-9a-f]+h$", token.value))
                    throw Exception{"Syntaxe error", "invalid token",&(tokens->back())};
            }
            else if (match("^[a-zA-Z0-9_]+$", token.value))
                tokens->back().type = IDENTIFIER;
            else if (match("^[:#.]$", token.value))
                tokens->back().type = SEPARATOR;
            else throw Exception{"Syntax error", "unidentified token", &(tokens->back())};
        }
    }
    return tokens;
}

/*Get the next token from the source code */
Token getNextToken(string_view src, Position *pos) {
    pos->column++;
    Token token{UNKNOW, "", pos->line, pos->column};
    char c = src.at(pos->index);
    if (c==' '||c=='\n'||c=='\t') {
        pos->index++;
        if (c =='\n') { pos->line++; pos->column = 1;}
    } else if (isAlphanumerical(c)) { // alphanumerical characters and underscore
        do {
            token.value += c;
            pos->index++; pos->column++;
            if (pos->index >= src.size()) break;
            c = src.at(pos->index);
        } while (isAlphanumerical(c));
        pos->column--;
    } else if (c == ';') { // comment
        if (pos->column == 1) token.value = "0";
        do {
            pos->index++;
            if (pos->index >= src.size()) break;
            c = src.at(pos->index);
        } while (c != '\n');
    } else if (c == '`') { // character
        token.type = ADDRESS;
        token.value = getSurroundedBy(c, src, pos);
    } else if (c == '\'') { // character
        token.type = CHAR;
        token.value = getSurroundedBy(c, src, pos);
    } else if (c == '\"') { // string
        token.type = STRING;
        token.value = getSurroundedBy(c, src, pos);
    } else { pos->index++; token.value = c; }
    return token;
}

string getSurroundedBy(char ch, string_view src, Position *pos) {
    string text;
    pos->index++;
    char c = src.at(pos->index);
    char n = 0, p = 0;
    while (c != ch) {
        pos->index++;
        pos->column++;
        if (pos->index >= src.size()) throw Exception{"Syntaxe error", "unterminated quote", nullptr};
        if (c =='\n') { pos->line++; pos->column = 1; }
        n = src.at(pos->index);
        if (c == '\\') {
            switch (n) {
                case 'n': text+='\n'; break;
                case 't': text+='\t'; break;
                case 'r': text+='\r'; break;
                default: text+=c; break;
            }
            pos->index++;
            pos->column++;
            if (pos->index >= src.size()) throw Exception{"Syntaxe error", "unterminated quote", nullptr};
            n = src.at(pos->index);
        } else
            text += c;
        p = c;
        c = n;
    }
    pos->index++;
    pos->column++;
    return text;
}

bool isAlphanumerical(char c) {
    return (65 <= c && c <= 90) || (97 <= c && c <= 122) || c == '_' || isDigit(c);
}
bool isDigit(char c) { return 48 <= c && c <= 57; }

bool match(const string& pattern, const string& _target) {
    const char *target = _target.c_str();
    regex rgx(pattern);
    auto iter = cregex_iterator(target, target+strlen(target), rgx);
    auto end = cregex_iterator();
    int i;
    for (i = 0; iter != end; ++iter, i++) {
        iter->str();
    }
    return i > 0;
}

void printToken(Token token, string_view end) {
    cout << token.type <<' '<< token.value << " at(" << token.line <<':'<< token.column <<')'<< end;
}

void printException(Exception e) {
    cout << e.type;
    if (e.token != nullptr)
        cout << " on token " << e.token->value << " at (" << e.token->line << ":" << e.token->column << ")";
    cout << " > " << e.message << endl;
}
