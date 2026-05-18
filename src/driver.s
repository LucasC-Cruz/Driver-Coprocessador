.equ SYSCALL_OPEN,      5
.equ SYSCALL_CLOSE,     6
.equ SYSCALL_MUNMAP,    91
.equ SYSCALL_MMAP2,     192

.equ PAGINA,            0x5000
.equ LWHPS2FPGA_BASE,   0xFF200

.equ PIO_INSTRUCTION,   0x00 
.equ PIO_ENABLE,        0x10 

.equ PIO_RESULTADO,     0x20
.equ PIO_FLAG_DONE,     0x30   
.equ PIO_FLAG_BUSY,     0x40   
.equ PIO_FLAG_ERROR,    0x50   
.equ PIO_CLR_OP,        0x60
.equ PIO_RESET_COP,     0x70
.equ PIO_CONFIRMAR,     0x80

.equ SYSCALL_EXIT,     1
.equ SYSCALL_READ,     3
.equ SYSCALL_WRITE,    4
.equ SYSCALL_OPEN,     5
.equ SYSCALL_CLOSE,    6
.equ READ_ONLY,        0


.section .bss
.align 2
    image_buffer:   .skip 784        @abre 784 bytes de espaço
    bias_buffer:    .skip 256         @128(bias)*2
    beta_buffer:    .skip 2560       @1280 betas
    pesos_buffer:   .skip 200704 

.section .data
dev_mem: .asciz "/dev/mem"
image_filename: .asciz "/home/aluno/TEC499/TP02/G0Paulo/Driver-Coprocessador/data/binImg/imagem_4.bin"
bias_filename:  .asciz "/home/aluno/TEC499/TP02/G0Paulo/Driver-Coprocessador/data/b_q_invertido.bin"
betas_filename: .asciz "/home/aluno/TEC499/TP02/G0Paulo/Driver-Coprocessador/data/beta_q_invertido.bin"
pesos_filename: .asciz "/home/aluno/TEC499/TP02/G0Paulo/Driver-Coprocessador/data/W_in_invertido.bin"
sucesso_leitura: .ascii "Consegui ler!\n"
mensagem_falha: .ascii "ERRO! Encerrando programa!"


.section .text
@ ==================== MAPEAMENTO ====================
@mapea memoria
@separar em dois (abrir e mapear) para permitir fechar o arquivo de devmem que é aberto!!
.global mapear
.type mapear, %function
mapear:
    push {r4, r5, r7, lr}    
    ldr r0, =dev_mem
    ldr r1, =0x101002           @O_RDWR|O_SYNC
    mov r7, #SYSCALL_OPEN
    svc 0                       @r0 contera fd
    @colocar tratamento de erro aqui

    mov r4, r0                  @fd da abertura para mapeamento
    mov r0, #0                  @endereço para o mapeamento
    ldr r1, =PAGINA
    mov r2, #3                  @PROT_READ | PROT_WRITE
    mov r3, #1                  @MAP_SHARED
    ldr r5, =LWHPS2FPGA_BASE    
    mov r7, #SYSCALL_MMAP2
    svc 0

    pop {r4, r5, r7, pc}

@fecha endereço mapeado
.global fechar
.type fechar, %function
fechar:
    push {r7, lr}
    mov r1, #PAGINA
    mov r7, #SYSCALL_MUNMAP
    svc 0
    pop {r7, pc}

@ ==================== SINAIS DE CONTROLE ==================== 

@inicia a execução da instrução enviada
.global enable
.type enable, %function
enable:
    @r0 deve ser o hps_virtual
    push {r1, lr}
    mov r1, #1
    str r1, [r0, #PIO_ENABLE]
    mov r1, #0
    str r1, [r0, #PIO_ENABLE]
    pop {r1, pc}

@reseta coprocessador
.global reset
.type reset, %function
reset:
    push {r1, lr}
    mov r1, #1
    str r1, [r0, #PIO_RESET_COP]
    mov r1, #0
    str r1, [r0, #PIO_RESET_COP]
    pop {r1, pc}

@limpa operação de erro 
.global clear_operation
.type clear_operation, %function
clear_operation:
    push {r1, lr}

    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    
    pop {r1, pc}

@ ==================== INSTRUÇÕES BÁSICAS ==================== 

@envia o valor de passado em r1 para o pio de instrução 
.global instruction
.type instruction, %function
instruction:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

@retorna a instrução que acabou de ser enviada
.global confirmar
.type confirmar, %function
confirmar:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    ldr r0, [r0, #PIO_CONFIRMAR]
    bx lr

@inicia inferencia
.global iniciar
.type iniciar, %function
iniciar:
    push {lr}
    @r0 deve ser o hps_virtual
    mov r1, #5
    str r1, [r0, #PIO_INSTRUCTION]
    bl enable
    bl espera_done
    pop {pc}

@não utilizada no coprocessador
.global status
.type status, %function
status:
    @r0 deve ser o hps_virtual
    mov r1, #6
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

@nop
.global NO_OP
.type NO_OP, %function
NO_OP:
    @r0 deve ser o hps_virtual
    mov r1, #7
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

@retorna resultado da inferencia
.global get_resultado
.type get_resultado, %function
get_resultado:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_RESULTADO]
    bx lr

@ ==================== INSTRUÇÔES DE MEMORIA ===================
@guarda pixel na memoria 
.global str_img
.type str_img, %function
str_img:
    @r0 deve ser o hps_virtual
    @ bl clear_operation              @abaixa a flag de done, colocar no inicio em vez daqui? permitiria conferir a flag no local que fez a chamada  
    push {r1, lr}       @push r1 para não alterar endereco que sera incrementado depois
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #13     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    pop {r1, pc}

@guarda bias na memoria 
.global str_bias
.type str_bias, %function
str_bias:
    @r0 deve ser o hps_virtual
    @bl clear_operation
    push {r1, r2, lr}       @push r1 para não alterar endereco que sera incrementado depois
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    cmp r2, #128
    bge finalizar_erro
    lsl r2, r2, #10     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    add r1, r1, #3      @soma op code de store bias
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    pop {r1, r2, pc}

@guarda beta na memoria 
.global str_beta
.type str_beta, %function
str_beta:
    @r0 deve ser o hps_virtual
    push {r1, lr}        @push r1 para não alterar endereco que sera incrementado depois
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #14     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    add r1, r1, #4      @soma op code de store beta
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    pop {r1, pc}

@guarda endereco do peso a ser enviado a memoria
@ endereço passado em r1 !! não há dado para esta funcao!!
.global str_wadress
.type str_wadress, %function
str_wadress:
    @r0 deve ser o hps_virtual
    push {r1, lr}       @push r1 para não alterar endereco que sera incrementado depois
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    add r1, r1, #1      @soma op code de store weigth adress
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    pop {r1, pc}                           @não tem done, então só volta direto...

@guarda peso na memoria no endereco enviado anteriormente
@ dado é passado em r2!! não há endereço para esta função!!
.global str_weight
.type str_weight, %function
str_weight:
    @r0 deve ser o hps_virtual
    push {lr}
    lsl r2, r2, #3      @r1 agr tem o dado no campo correto 
    add r2, r2, #2      @soma op code de store weigth
    str r2, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio       
    pop {pc}

@ ==================== FLAGS ==================== 

@espera ocupada para aguarda o sinal de done de um instrução enviada
@ usar apenas em stores loops, retorna em r2!!!
.global espera_done
.type espera_done, %function
espera_done:
    push {r0, r1, lr}
espera:
    ldr r2, [r0, #PIO_FLAG_ERROR]
    cmp r2, #0                      @compara errro com 0
    bne fim                        @se houve erro

    ldr r1, [r0, #PIO_FLAG_DONE]    @guarda o sinal de done em r1
    cmp r1, #0                      @compara done com 0
    beq espera                      @se é zero, lê done de novo, até ser 1   
fim:
    pop {r0, r1, pc}                           @retorna

@retorna flag de done
.global get_flag_done
.type get_flag_done, %function
get_flag_done:
    @r0 deve ser o hps_virtual
    push {lr}
    ldr r0, [r0, #PIO_FLAG_DONE]
    pop {pc}

@retorna flag de busy
.global get_flag_busy
.type get_flag_busy, %function
get_flag_busy:
    @r0 deve ser o hps_virtual
    push {lr}
    ldr r0, [r0, #PIO_FLAG_BUSY]
    pop {pc}

@retorna flag de erro
.global get_flag_error
.type get_flag_error, %function
get_flag_error:
    @r0 deve ser o hps_virtual
    push {lr}
    ldr r0, [r0, #PIO_FLAG_ERROR]
    pop {pc}


@ =================================== STORE ON LOOP =====================================

@recebe como entrada no r0, o hps virtual 
@tem como saida nada
.global store_image
    .type store_image, %function
    store_image:
        push {r1-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario      
        ldr r0, =image_filename
        bl abrir_arquivo
        mov r6, r0                   @ guarda fd em r6 para fechar depois

        mov r7, #SYSCALL_READ       
        ldr r1, =image_buffer     
        mov r2, #784              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r3, =image_buffer         @ponteiro do buffer
        mov r1, #0                  @ contador

        mov r0, r8  @hps virtual, necessario para a chamada

    store_image_loop:
        ldrb r2, [r3], #1   @lendo o valor do buffer para r1, e incrementa ponteiro

        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço, que está e atualizado em r5
        bl str_img @guarda no pio ins
        bl enable
        bl espera_done

        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #784
        bne store_image_loop
    
    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r1-r8, pc}


@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_bias
    .type store_bias, %function
    store_bias:
        push {r1-r8, lr}
        
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        ldr r0, =bias_filename
        bl abrir_arquivo
        mov r6, r0                   @ guarda fd em r6 para fechar depois


        mov r7, #SYSCALL_READ       
        ldr r1, =bias_buffer     
        mov r2, #128              @ Quantos bytes vai ler
        svc #0      
                        @ Buffer foi preenchido com dados do arquivo
        

        ldr r3, =bias_buffer         @ponteiro do buffer
        mov r1, #0                  @ contador

        mov r0, r8  @hps virtual, necessario para a chamada

    store_bias_loop:
        ldrh r2, [r3]
        add r3, r3, #2   @lendo o valor do buffer para r2, e incrementa ponteiro

        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço
        bl str_bias @guarda no pio ins
        bl enable
        bl espera_done
        cmp r2, #0
        bne finalizar_erro
        
        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #130
        bne store_bias_loop

    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r1-r8, pc}

@função para armazaenar toda os betas
@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_beta
    .type store_beta, %function
    store_beta:
        push {r1-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        ldr r0, =betas_filename
        bl abrir_arquivo
        mov r6, r0                   @ guarda fd em r6 para fechar depois

        mov r7, #SYSCALL_READ       
        ldr r1, =beta_buffer     
        mov r2, #2560              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r3, =beta_buffer         @ponteiro do buffer
        mov r1, #0                  @ contador

        mov r0, r8  @hps virtual, necessario para a chamada

    store_beta_loop:
        ldrh r2, [r3]   @lendo o valor do buffer para r1, e incrementa ponteiro
        add r3, r3, #2
        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço, que está e atualizado em r5
        bl str_beta @guarda no pio ins
        bl enable
        bl espera_done
        cmp r2, #0
        bne finalizar_erro
        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #1280
        bne store_beta_loop
    
    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r1-r8, pc}

@função para armazaenar toda os betas
@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_pesos
    .type store_pesos, %function
    store_pesos:
        push {r1-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        ldr r0, =pesos_filename
        bl abrir_arquivo
        mov r6, r0                   @ guarda fd em r6 para fechar depois

        mov r7, #SYSCALL_READ       
        ldr r1, =pesos_buffer     
        mov r2, #200704              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r3, =pesos_buffer         @ponteiro do buffer
        mov r1, #0                  @ contador (não r1, pois é usado como dado em)

        mov r0, r8  @hps virtual, necessario para a chamada

    store_pesos_loop:
        ldrh r2, [r3]   @lendo o valor (half word) do buffer para r1, e incrementa ponteiro por 2 (bytes)
        add r3, r3, #2
        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço, que está e atualizado em r5
        bl str_wadress  @guarda o endereco r1
        bl enable

        bl str_weight @possiblidade de apatar str_weight para usar r2, retirando mov  
        bl enable
        bl espera_done
        cmp r2, #0      @retorno de espera_done diz se houve erro ou não
        bne finalizar_erro

        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #100352
        bne store_pesos_loop
    
    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r1-r8, pc}


.global sucesso
.type sucesso, %function
sucesso:
    
    push {r1, lr}
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =sucesso_leitura
    mov r2, #17            @tamanho da saída em bytes
    svc #0
    pop {r1, pc}



.global falha
.type falha, %function
falha:
    
    push {r1, lr}
    mov r7, #SYSCALL_WRITE @escreve
    mov r0, #1             @stdout (tela)
    ldr r1, =mensagem_falha
    mov r2, #26            @tamanho da saída em bytes
    svc #0
    pop {r1, pc}
    


@espera endereco do arquivo em r0
.global abrir_arquivo
.type abrir_arquivo, %function
    abrir_arquivo:

        mov r7, #SYSCALL_OPEN       
        @ldr r0, =x_filename
        mov r1, #READ_ONLY          
        svc #0    
        bx lr

@fechar arquivoa apush {r0, lr}berto
.global fechar_arquivo
.type fechar_arquivo, %function
    fechar_arquivo:
        mov r7, #SYSCALL_CLOSE
        svc 0
        bx lr

.global finalizar_erro
.type finalizar_erro, %function
    finalizar_erro:
        bl falha
        mov r7, #1
        mov r0, #1
        svc #0
