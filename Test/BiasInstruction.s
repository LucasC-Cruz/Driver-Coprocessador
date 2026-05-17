



.equ SYSCALL_EXIT,     1
.equ SYSCALL_OPEN,     5
.equ SYSCALL_READ,     3
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_WRITE,    4

.extern mapear
.extern reset
.extern instruction
.extern iniciar
.extern get_resultado
.extern status
.extern get_flag_done
.extern get_flag_busy
.extern get_flag_error
.extern enable
.extern fechar
.extern print_reg
.extern print_msg


@TODO: 
@ - Abertura de buffer com tamanho dinâmico??
@ - Seria bom persistirmos o número de vezes que executamos a inferência
@
@
@



.section .bss
    ins: .skip 20     @abre 16 bytes de espaço, quero fazer isto dinâmicamente
    saida: .skip 100  @valor qqlr aq só pra ter um buffer de saída

.section .data
    filename:        .asciz "output.bin"
    error_leitura:   .ascii "Erro na leitura\n"
    msg_g:           .ascii "O valor é: "
    sucesso_leitura: .ascii "Consegui ler!\n"


.section .text



.global print_msg

@ENTRADA: ponteiro de buffer com mensagem
@SAÍDA: r0 com o valor de números bytes realmente escritos
.type print_msg, %function
print_msg:
    push {r0, r1, r2, r7, lr}
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)

    @printa a primeira mensagem
    ldr r1, =msg_g
    mov r2, #11            @tamanho da saída em bytes
    svc #0

    pop {r0, r1, r2, r7, pc}

.global print_reg 
@ENTRADA: R0: com valor a ser exibido
@         endereço do buffer de saída
@SAÍDA:   R0: com valor de bytes realmente escritos
.type print_reg, %function
print_reg:
    push {r1, r2, r7, r9, r10, lr}
    mov r10, r0
    ldr r9, =saida
    add r0, r0, #48
    strh r0, [r9]
    

    
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)

    @printa o valor do buffer saida
    ldr r1, =saida
    mov r2, #10            @tamanho da saída em bytes
    svc #0

    mov r0, r10
    pop {r1, r2, r7, r9, r10, pc}



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

.type converte_decimal, %function
    converte_decimal:
        sub r0, r0, #48
        bx lr

@r0 agr tem o valor de resultado


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
        mov r2, #6      @r0 agr tem o valor de resultado              @ Quantos bytes ele vai ler
        svc #0


                                       @ r0 = (você guarda o resultado aqui)
                                       @ r0, contém quantas merdas foram lidas com sucesso
        cmp r0, #6                     @ se leu menos do devia ent é pra dar erro
        blt erro
        bl sucesso
        pop {r1, r2, r7, pc}
@cospe o r5 e o r0 com o endereço para leitura




    @seguinte, precisamos fazer as intruções do bias e mandar para a placa
    @o fluxo de execução será controlado pelas flags da placa para garantirmos sincronização
    @ler do arquivo o primeiro binário, 16bits
    @somar com esses 16 bits o opcode não shiftado
    @somar com esses 16 bits o endereço shiftado pra esquerda por x
    @o loop vai contar de 0 - n, todo valor será shiftado para a esquerda antes
    @depois de somar geral chamamos as funções que mandam para a placa e reiniciamos o loop até preencher a memória


@r0 agr tem o valor de resultado
.global main
main:

    mov r10, #1

    @validações mapear
    


    bl open
    bl read

    bl mapear @r0 tem endereço base agora
    @bl print

    mov r10, r0 @salvando em r10 para podermos utilizar no futuro
    mov r9, #0
    
@buffer já cheio dos binários em sequência
@atualmente tem vários lixos aí
    ldr r5, =ins  
    mov r6, #3 @hardcoded por enquanto
    b loop

    b finalizar_sucesso



loop:
    b send_bias
    cmp r9, r6
    blt loop
    b finalizar_sucesso


send_bias:
    ldrh r1, [r5, r9] @carrega 2 bytes do buffer ins (half-word)

    lsl r1, #3      @dá offset dado para garantir que n vai somar de fato com o opcode

    mov r4, r9      @pega endereço atual
    lsl r4, #10     @offset de 10

    @adiciona opcode
    add r1, r1, #3  @store_bias 011
    @adiciona endereço
    add r1, r1, r4


    @r1 agora possui a instrução completa
   @ bl print    

   
    @r0, com endereço base
    
    @recebe r1 com a instrução
    @recebe r0 com o hps_virtual
    bl instruction@r0 agr tem o valor de resultado

    @o r1 perde a instrução aqui, mas não utilizamos novamente até a próxima instrução
    bl enable

    mov r0, r10
    
    bl get_flag_busy
    @r0 agr tem o valor de busy
    bl print_msg
    bl print_reg

   @ bl print
    @escrever para debug

    mov r0, r10
    bl get_flag_error
    @r0 agr tem o valor de error
    bl print_msg
    bl print_reg
    
  
    mov r0, r10
    bl get_flag_done
    @r0 agr tem o valor de done
    bl print_msg
    bl print_reg
    

    add r9, r9, #1 @incrementa contador
    cmp r0, #1
    beq loop





    


