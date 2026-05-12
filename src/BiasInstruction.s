



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



@TODO: 
@ - Abertura de buffer com tamanho dinâmico
@ - Montar instrução
@ - Seria bom persistirmos o número de vezes que executamos a inferência
@
@
@



.section .bss
    ins: .skip 5     @abre 16 bytes de espaço, quero fazer isto dinâmicamente
    saida: .skip 10  @valor qqlr aq só pra ter um buffer de saída

.section .data
    filename:        .asciz "/mnt/c/Users/lucas/Desktop/Programação/PastasVs/Assembly/Driver-Coprocessador/data/TesteBias.txt"
    error_leitura:   .ascii "Erro na leitura\n"
    msg_g:           .ascii "O valor é: "
    sucesso_leitura: .ascii "Consegui ler!\n"


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
    

print:
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)

    @printa a primeira mensagem
    ldr r1, =msg_g
    mov r2, #11            @tamanho da saída em bytes
    svc #0

    @printa o valor do buffer saida
    ldr r1, =saida
    mov r2, #10            @tamanho da saída em bytes
    svc #0

    bx lr
 
escreve_b_s:
    push {r9, lr}
    ldr r9, =saida
    add r0, r0, #48
    str r0, [r9]
    pop {r9, pc}



@basicamente tem uma receitinha de bolo para abrir um arquivo
.type open, %function
    open:
        mov r7, #SYSCALL_OPEN          @ open
        ldr r0, =filename
        mov r1, #0          @flag que indica que eu to abrindo o arquivo só para ler
        svc #0              @ retorna fd em r0 (Por que syscalls devolvem valores por padrão em r0)
                            @Em outras funções      
        bx lr

@cospe o r0 com fd


@O read lê todo o buffer e sobreescreve por padrão 
.type read, %function
    read:

        push {r1, r2, r7, lr}
        mov r7, #SYSCALL_READ          @ read
        mov r0, r0                     @ r0, r0 = file descriptor se esta merda estiver pós 
                                       @ abertura de arquivo a ordem importa

        ldr r1, =ins                   @ neste caso usa r1 pra receber o buffer
        mov r2, #5                    @ Quantos bytes ele vai ler
        svc #0


                                       @ r0 = (você guarda o resultado aqui)
                                       @ r0, contém quantas merdas foram lidas com sucesso
        cmp r0, #5                     @ se leu menos do devia ent é pra dar erro
        blt erro
        bl sucesso
        pop {r1, r2, r7, pc}
@cospe o r5 e o r0 com o endereço para leitura

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




    @seguinte, precisamos fazer as intruções do bias e mandar para a placa
    @o fluxo de execução será controlado pelas flags da placa para garantirmos sincronização
    @ler do arquivo o primeiro binário, 16bits
    @somar com esses 16 bits o opcode não shiftado
    @somar com esses 16 bits o endereço shiftado pra esquerda por x
    @o loop vai contar de 0 - n, todo valor será shiftado para a esquerda antes
    @depois de somar geral chamamos as funções que mandam para a placa e reiniciamos o loop até preencher a memória


.section .text
.global _start

_start:
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

    @validações mapear
    


    bl open
    bl read

    bl mapear @r0 tem endereço base agora

    mov r10, r0

@buffer já cheio dos binários em sequência
    ldr r5, =ins  
    mov r6, #5 @hardcoded por enquanto
    b loop

    b finalizar_sucesso



loop:
    add r9, r9, #1 @incrementa contador
    b send_bias
    cmp r9, r6
    blt loop
    b finalizar_sucesso


send_bias:
    ldrh r1, [r5, r9] @carrega 2 bytes half-word

    lsl r1, #3      @dá offset dado para garantir que n vai somar de fato com o opcode

    mov r4, r9      @pega endereço atual
    lsl r4, #10     @offset de 10

    @adiciona opcode
    add r1, r1, #3  @store_bias 011
    @adiciona endereço
    add r1, r1, r4


    @r1 agora possui a instrução completa
    mov r0, r1
    bl escreve

    mov r0, r10
    @r0, com endereço base
    
    bl store

    bl pulso_enable

    
    bl resultado
    @r0 agr tem o valor de resultado
    bl escreve
    @escrever para debug

    bl flag_busy
    @r0 agr tem o valor de busy
    bl escreve
    @escrever para debug

    bl flag_error
    @r0 agr tem o valor de error
    bl escreve
    @escrever para debug

    bl flag_done
    @r0 agr tem o valor de done
    bl escreve
    @escrever para debug
    cmp r0, #1
    beq loop





    


