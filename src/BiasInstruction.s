



.equ SYSCALL_EXIT,     1
.equ SYSCALL_OPEN,     5
.equ SYSCALL_READ,     3
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_WRITE,    4

.extern mapear
.extern reset
.extern iniciar
.extern status
.extern resultado
.extern flag_done
.extern flag_busy
.extern flag_error
.extern enable
.extern pulso_enable
.extern fechar


.section .bss
    ins: .skip 16     @queria manipular bit a bit porque isto daqui são 4 bytes, correto?
    

.section .data
    filename:        .asciz "/mnt/c/Users/lucas/Desktop/Programação/PastasVs/Assembly/Driver-Coprocessador/data/TesteBias.txt"
    error_leitura:   .ascii "Erro na leitura\n"
    sucesso_leitura: .ascii "Consegui ler!\n"



.section .text
.global _start



    @basicamente tem uma receitinha de bolo para abrir um arquivo
.type open, %function
    open:
        

        mov r7, #SYSCALL_OPEN          @ open
        ldr r0, =filename
        mov r1, #0          @flag que indica que eu to abrindo o arquivo só para ler
        svc #0              @ retorna fd em r0 (Por que syscalls devolvem valores por padrão em r0)

        mov r4, r0          @ r4 agora contém o fd (ex: 3, 4, 5...) Que será usado para manipular este arquivo e
                            @Em outras funções

        
        bx lr

.type read, %function
@O read lê todo o buffer e sobreescreve por padrão 
@para uma unidade de dado tá tudo bem
@porém para contruirmos várias instruções e considerando que ele não "pula" para uma parte específica do documento e 
@sim lê sempre sequencialmente a partir do primeiro rapaz, deveremos fazer alguma lógica aí para ler vários carinhas
@e pegar o dado correto para construirmos a nossa instrução (o próximo) sem dar overhead

@Talvez o correto seja ler somente uma vez? e guardar tudo em um buffer?
@tem como eu pular? Eu só assumi que não dava
    read:

        push {r0, r1, r2, r4, r7, lr}
        mov r7, #SYSCALL_READ          @ read
        mov r0, r4           @ r0, r4 = file descriptor
        ldr r1, =ins         @neste caso usa r1 pra receber o buffer
        mov r2, #16           @Quantos bytes ele vai ler
        svc #0

        mov r5, r0           @ r5 = (você guarda o resultado aqui)
        cmp r5, #2
        blt erro
        bl sucesso
        pop {r0, r1, r2, r4, r7, pc}


.type finalizar_sucesso, %function
    finalizar_sucesso:
        mov r7, #1
        mov r0, #0
        svc #0

.type finalizar_erro, %function
    finalizar_erro:
        mov r7, #1
        mov r0, #1
        svc #0

.type converte_ascii, %function
    converte_decimal:
        sub r0, r0, #48
        bx lr

erro:

    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =error_leitura
    mov r2, #20            @tamanho da saída em bytes
    svc #0                
    b finalizar_erro      


sucesso:

    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =sucesso_leitura
    mov r2, #17            @tamanho da saída em bytes
    svc #0
    bx lr
    

escreve:
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =ins
    mov r2, #16            @tamanho da saída em bytes
    svc #0
    bx lr
 

converte_bin:
    lsl r8, r10, r3
    add r0, r8
    b loop

@Vou considerar para este código o r0, r1, r2, r3, parâmetros para as funções e o r0 também será  utilizado para  receber os retornos das funções
_start:

    @queremos ler um binário de um arquivo
    @queremos pegar este binário e formar a instrução
    @a instrução é formada por
    @ 31 -> 0
    @ 31 -> 26 nonp
    @ 25 -> 10 dado (16 bits)
    @ 9  -> 3 endereço (inicial de 0000000)
    @ 2  -> 0 OPCODE (011)
    mov r0,  #0
    mov r1,  #0
    mov r2,  #0
    mov r3,  #0
    mov r4,  #0
    mov r5,  #0
    mov r6,  #0
    mov r7,  #0
    mov r8,  #0
    mov r9,  #0
    mov r10, #1

    bl open
    bl read

    ldr r1, =ins  
    mov r3, #15




    @basicamente vamos ler os acii e pegar os bits menos significativos usando alguma função de merda aí
    @depois atribuiremos a um registrador estes bits
    bl escreve
    b finalizar_sucesso

loop:
    ldrb r4, [r1, r9]
    sub r3, r3, #1
    and r4, r4, #1
    cmp r4, #1  
    beq converte_bin
    cmp r3, #0
    beq finalizar_sucesso


    @uma ideia para converter o ascii para os bits 
    @ somar
    @vamos contar as posições do ascii, se eum posição for 1
    @somamos a um registrador 2^n sendo n a posição dele no "binário"





    


