## Documentation
See DOC.md for the full project's architecture

## Dependencies
Any C++ compiler

## Compilation 
The Each folder has to be compiled separatly.
```bash
g++ ./*.cpp -o <target>
```
should work just fine.

## Running
```bash
Compiler <source> [dest]
```
to compile from assemble and then
```bash
VirtualMachine <binary>
```
to run (where 'Compiler' and 'VirtualMachine' are the executables).


There is still no doccumentation for the assembly language, feel free to use my example code as a guide.
