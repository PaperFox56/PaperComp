#include <stdio.h>
#include <cpu.h>

using namespace std;

void loadROM(u_int16_t *mem, const char *path);

int main(int count, char **args) {
    CPU *cpu = new CPU();
    loadROM(cpu->mem, args[1]);
    cpu->init();
    // Load the arguments in the first 255 blocks of the memory
    int i, j;
    for (i=2, j=count-1; i<count and j<256; i++) {
        string temp = args[i];
        cpu->mem[i-1] = j;
        for (int k = 0; k<temp.size()+1 and j<256; k++, j++) {
            cpu->mem[j] = k<temp.size() ? temp[k] : 0;
        }
    }
    cpu->mem[0] = i-2;
    cout << "\n--- PaperComp inited ---\n--- Program loaded ---\n" << endl;
    while (cpu->isRuning) {
        cpu->interpreteNextOpcode();
    }
    return 0;
}

void loadROM(u_int16_t *mem, const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == nullptr) throw "Unable to open the file";
    int n = 0;
    fread(&n, 2, 1, file);
    fread(mem, 2, n-1, file);
    fclose(file);
    int i = 1;
}
