
@Declaração de Constantes

.equ SYSCALL_EXIT,     1
.equ SYSCALL_OPEN,     5
.equ SYSCALL_READ,     3
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_WRITE,    4





.section .bss
    .global saida
    saida: .skip 100  @valor qqlr aq só pra ter um buffer de saída

.section .data
    filename:        .asciz "/mnt/c/Users/lucas/Desktop/Programação/PastasVs/Assembly/Driver-Coprocessador/data/TesteBias.txt"
    error_leitura:   .ascii "Erro na leitura\n"
    msg_g:           .ascii "O valor é: "
    sucesso_leitura: .ascii "Consegui ler!\n"


        

.global print_msg
.type print_msg, %function
print_msg:

    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)

    @printa a primeira mensagem
    ldr r1, =msg_g
    mov r2, #11            @tamanho da saída em bytes
    svc #0

    bx lr
 
 
.type print_reg, %function
print_reg:
    ldr r9, =saida
    strb r0, [r9]

    
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)

    @printa o valor do buffer saida
    ldr r1, =saida
    mov r2, #4            @tamanho da saída em bytes
    svc #0

    bx lr
