



.equ SYSCALL_EXIT,     1
.equ SYSCALL_OPEN,     5
.equ SYSCALL_READ,     3
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_WRITE,    4

.section .bss
    ins: .skip 4     @queria manipular bit a bit porque isto daqui são 4 bytes, correto?
    

.section .data
    filename: .asciz "TesteBias.txt"
    error_leitura: .ascii "Erro na leitura\n"



.section .text
.global _start



    @basicamente tem uma receitinha de bolo para abrir um arquivo
.type open, %function
    open:
        push {r0, r1, r7, r4, lr}

        mov r7, #SYSCALL_OPEN          @ open
        ldr r0, =filename
        mov r1, #0          @flag que indica que eu to abrindo o arquivo só para ler
        svc #0              @ retorna fd em r0 (Por que syscalls devolvem valores por padrão em r0)

        mov r4, r0          @ r4 agora contém o fd (ex: 3, 4, 5...) Que será usado para manipular este arquivo e
                            @Em outras funções

        pop {r0, r1, r7, pc}

.type read, %function
    read:
        push {r0, r1, r2, r3, r4, lr}
        mov r7, #SYSCALL_READ          @ read
        mov r0, r4           @ r0, r4 = file descriptor
        ldr r1, =ins         @neste caso usa r1 pra receber o buffer
        mov r2, #2           @Quantos bytes ele vai ler
        svc #0

        mov r5, r0           @ r5 = 50 (você guarda o resultado aqui)
        cmp r5, #2
        blt erro
        pop {r0, r1, r2, r3, pc}

erro:

    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =error_leitura
    mov r2, #17            @tamanho da saída em bytes
    svc #0  


_start:

    @queremos ler um binário de um arquivo
    @queremos pegar este binário e formar a instrução
    @a instrução é formada por
    @ 31 -> 0
    @ 31 -> 26 nonp
    @ 25 -> 10 dado (16 bits)
    @ 9  -> 3 endereço (inicial de 0000000)
    @ 2  -> 0 OPCODE (011)




    


