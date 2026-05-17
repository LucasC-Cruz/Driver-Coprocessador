.extern str_img
.extern str_bias
.extern str_beta
.extern str_wadress
.extern str_weigth

.extern enable
.extern espera_done

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
    pesos_buffer:   .skip 200704     @100.352 pesos. pequeno esse, será que dá problema? são 0,2 mb

.section .data
    image_filename: .asciz "Driver-Coprocessador/data/imagem_4.bin"
    bias_filename:  .asciz "Driver-Coprocessador/data/b_q_invertido.bin"
    betas_filename: .asciz "Driver-Coprocessador/data/beta_q_invertido.bin"
    pesos_filename: .asciz "Driver-Coprocessador/data/W_in_invertido.bin"

@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.section .text

@recebe como entrada no r0, o hps virtual 
@tem como saida nada
.global store_image
    .type store_image, %function
    store_image:
        push {r0-r8, lr}
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
        cmp r2, #0
        bne finalizar_erro

        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #784
        bne store_image_loop
    
    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r0-r8, pc}


@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_bias
    .type store_bias, %function
    store_bias:
        push {r0-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        ldr r0, =bias_filename
        bl abrir_arquivo
        mov r6, r0                   @ guarda fd em r6 para fechar depois

        mov r7, #SYSCALL_READ       
        ldr r1, =bias_buffer     
        mov r2, #256              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r3, =bias_buffer         @ponteiro do buffer
        mov r1, #0                  @ contador

        mov r0, r8  @hps virtual, necessario para a chamada

    store_bias_loop:
        ldrh r2, [r3], #2   @lendo o valor do buffer para r2, e incrementa ponteiro

        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço
        bl str_bias @guarda no pio ins
        bl enable
        bl espera_done @retorna em r2!!
        cmp r2, #0
        bne finalizar_erro
        @ adicionar retorno em espera done indicando tbm erro
        @ e entao uma comparação aqui para tratar erro
        add r1, #1
        cmp r1, #128
        bne store_bias_loop
    
    @fechar
        mov r0, r6
        bl fechar_arquivo

        pop {r0-r8, pc}

@função para armazaenar toda os betas
@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_beta
    .type store_beta, %function
    store_beta:
        push {r0-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        ldr r0, =beta_filename
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
        ldrh r2, [r3], #2   @lendo o valor do buffer para r1, e incrementa ponteiro

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

        pop {r0-r8, pc}

@função para armazaenar toda os betas
@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.global store_pesos
    .type store_pesos, %function
    store_pesos:
        push {r0-r8, lr}
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
        ldrh r2, [r3], #2   @lendo o valor (half word) do buffer para r1, e incrementa ponteiro por 2 (bytes)

        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço, que está e atualizado em r5
        bl str_wadress  @guarda o endereco r1
        bl enable

        bl str_weight //possiblidade de apatar str_weight para usar r2, retirando mov
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

        pop {r0-r8, pc}


@espera endereco do arquivo em r0
.global abrir_arquivo
.type abrir_arquivo, %function
    abrir_arquivo:
        push {r0, r1, r7, lr}
            mov r7, #SYSCALL_OPEN       
            @ldr r0, =x_filename
            mov r1, #READ_ONLY          
            svc #0    
        pop {r0, r1, r7, pc}

@fechar arquivoa aberto
.global fechar_arquivo
.type fechar_arquivo, %function
    fechar_arquivo:
        push {r0, lr}
        mov r7, #SYSCALL_CLOSE
        svc 0
        pop {r0, pc}

.type finalizar_erro, %function
    finalizar_erro:
    
        mov r7, #1
        mov r0, #1
        svc #0