backToLine: ; this subroutine put a back to line caracter
            ld H 1
            ld G '\n'
            syscall
            ret

addVectors: ; load the vector addresses
            pop B
            pop A
            add C `A` `B`
            inc A
            inc B
            add D `A` `B`
            push D
            push C
            ret

subVectors: ; load the vector addresses
            pop B
            pop A
            sub C `A` `B`
            inc A
            inc B
            sub D `A` `B`
            push D
            push C
            ret

start__:
            ld A vector1
            ld C vector2
            mov B A
            mov D C
            ; push the addresses on the stack
            inc B
            inc D
            ; v1 = (4, 3)
            ld `A` 4
            ld `B` 3
            ; v2 = (2, 5)
            ld `C` 2
            ld `D` 5
            push A
            push C
            call addVectors
            pop B
            pop D
            call printVector
            push A
            push C
            call subVectors
            pop B
            pop D
            call printVector
            hlt

printVector:
            ld H 2
            ld G string
            syscall
            ld H 3
            mov G B
            syscall
            ld H 1
            ld G ' '
            syscall
            ld H 3
            mov G D
            syscall
            call backToLine
            ret


vector1: 0 0
vector2: 0 0
string: "The result vector has coordinates "
