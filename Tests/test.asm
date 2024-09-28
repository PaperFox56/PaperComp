backToLine: ; this subroutine put a back to line caracter
            ld H 1
            ld G '\n'
            syscall
            ret

start__:
            ld C `0`
            ld A 0
            ld H 2
            loop loop_
            jp end
    loop_:
            inc A
            mov G `A`
            ld H 2
            syscall
            call backToLine
            loop loop_
    end:
            mov G `A`
            hlt
