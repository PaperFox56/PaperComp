backToLine: ; this subroutine put a back to line caracter
            ld H 1
            ld G '\n'
            syscall
            ret

printResult:
            ld H 2
            ld G result
            syscall
            ret

Sub:
            call printResult
            pop B
            pop A
            sub C A B
            jinf l1
            jp l2
    l1:     ld H 1
            ld G '-'
            syscall
    l2:     ld H 3
            mov G C
            syscall
            call backToLine
            ret

Plus:
            call printResult
            pop B
            pop A
            add G A B
            ld H 3
            syscall
            call backToLine
            ret

Mul:
            call printResult
            pop B
            pop A
            mul G A B
            ld H 3
            syscall
            call backToLine
            ret

Div:
            call printResult
            pop B
            pop A
            div G A B
            ld H 3
            syscall
            call backToLine
            ret

Modulo:
            call printResult
            pop B
            pop A
            div G A B
            ld H 3
            mov G F
            syscall
            call backToLine
            ret

start__:
            ld H 2
            ld G info
            syscall
    begin:  ; ask for the operator
            ld H 2
            ld G opQuerry
            syscall
            ld H 5
            ld G operator
            syscall
            ; ask for the operands
            ld H 2
            ld G op1Querry
            syscall
            ld H 4
            syscall
            push G
            ld H 2
            ld G op2Querry
            syscall
            ld H 4
            syscall
            push G
            ; Now we check the operator and call the right subroutine
            ld A `operator`
            ld B '+'
            sub C A B
            jeq call_plus
            ld B '-'
            sub C A B
            jeq call_sub
            ld B '*'
            sub C A B
            jeq call_mul
            ld B '/'
            sub C A B
            jeq call_div
            ld B '%'
            sub C A B
            jeq call_mod
            ; at this stop the operator is invalid so we end the program
            jp stop
    call_sub:
            call Sub
            jp loop_
    call_plus:
            call Plus
            jp loop_
    call_mul:
            call Mul
            jp loop_
    call_div:
            call Div
            jp loop_
    call_mod:
            call Modulo
    loop_:  call backToLine
            jp begin
    stop:
            ld H 2
            ld G invalid
            syscall
            hlt

info:       "\nThis is the first PaperComp's calculator ever\nI'm too lazy to check the entries so don't be sad if it break.\n\n"
opQuerry:   "Operator: "
op1Querry:  "First operand: "
op2Querry:  "Second operand: "
result:     "The result is "
invalid:    "The operator is invalid.\n"

operator:   .fill 10
