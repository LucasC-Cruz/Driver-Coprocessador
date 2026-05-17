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


.section .data
dev_mem: .asciz "/dev/mem"

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
    bx lr
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
    push {r1, lr}       @push r1 para não alterar endereco que sera incrementado depois
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #10     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    add r1, r1, #3      @soma op code de store bias
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    pop {r1, pc}

@guarda beta na memoria 
.global str_beta
.type str_beta, %function
str_beta:
    @r0 deve ser o hps_virtual
    push {r1,lr}        @push r1 para não alterar endereco que sera incrementado depois
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
