#include <parser.h>
#include <fstream>

using namespace std;

string load(string path);
void store(u_int16_t *mem, char* path);
void storeAsm(string src, string path);

int main(int count, char **args) {
    string op = "-c";
    string src = "a = 'd'\nc = \"=a+\"";
    char *out = "out.fcomp";
    if (count > 2) op = args[1];
    if (count > 2) src = load(args[2]);
    if (count > 3) out = args[3];
    vector<Token> *tokens;
    try {
        tokens = tokenize(src);
        u_int16_t *mem = assemble(tokens);/*
        for (int i=0; i < tokens->size(); i++) {
            printToken(tokens->at(i), "\t");
        } cout<<endl;*/
        store(mem, out);
        free(mem);
    } catch (Exception e) {
        printException(e);
        return 1;
    }
    /*char c = ' ';
    cout << isAlphanumerical(c) <<'\t'<< int(c) << endl;*/
    return 0;
}

void store(u_int16_t *mem, char *path) {
    FILE *file = fopen(path, "wb+");
    fwrite(mem, 2, mem[0], file);
    fclose(file);
}
/*
void storeAsm(string src, string path) {
    ofstream file(path, "w+");
    if (!file) throw "Invalid path";
    file << src;
    file.close();
}*/

string load(string path) {
    ifstream file(path);
    if (!file) throw "Invalid path";
    string out;
    string temp;
    while (!file.eof()) {
        getline(file, temp);
        out += temp+'\n';
    }
    file.close();
    return out;
}
