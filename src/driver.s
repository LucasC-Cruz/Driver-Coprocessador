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

@ ==================== MAPEAMENTO ====================
.section .text
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


.global fechar
.type fechar, %function
fechar:
    push {r7, lr}
    mov r1, #PAGINA
    mov r7, #SYSCALL_MUNMAP
    svc 0
    pop {r7, pc}

@ ==================== SINAIS DE CONTROLE ==================== 

.global enable
.type enable, %function
enable:
    @r0 deve ser o hps_virtual
    push {lr}
    mov r1, #1
    str r1, [r0, #PIO_ENABLE]
    mov r1, #0
    str r1, [r0, #PIO_ENABLE]
    pop {pc}

.global reset
.type reset, %function
reset:
    mov r1, #1
    str r1, [r0, #PIO_RESET_COP]
    mov r1, #0
    str r1, [r0, #PIO_RESET_COP]
    bx lr

.global clear_operation
.type clear_operation, %function
clear_operation:
    push {lr}

    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    
    pop {pc}

@ ==================== INSTRUÇÕES BÁSICAS ==================== 

.global instruction
.type instruction, %function
instruction:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

.global confirmar
.type confirmar, %function
confirmar:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    ldr r0, [r0, #PIO_CONFIRMAR]
    bx lr

.global iniciar
.type iniciar, %function
iniciar:
    @r0 deve ser o hps_virtual
    mov r1, #5
    str r1, [r0, #PIO_INSTRUCTION]
    @ bl enable
    @ bl espera_done
    bx lr

.global status
.type status, %function
status:
    @r0 deve ser o hps_virtual
    mov r1, #6
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

.global NO_OP
.type NO_OP, %function
NO_OP:
    @r0 deve ser o hps_virtual
    mov r1, #7
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

.global get_resultado
.type get_resultado, %function
get_resultado:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_RESULTADO]
    bx lr

@ ==================== INSTRUÇÔES DE MEMORIA ===================
.global str_img
.type str_img, %function
str_img:
    @r0 deve ser o hps_virtual
    @ bl clear_operation              @abaixa a flag de done, colocar no inicio em vez daqui? permitiria conferir a flag no local que fez a chamada  
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #13     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    @ bl enable                       @enable na instrucao
    @ bl espera_done                  @agurda done para retorno, se der erro vai ficar preso pra sempre aqui...
    bx lr

.global str_bias
.type str_bias, %function
str_bias:
    @r0 deve ser o hps_virtual
    @bl clear_operation
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #10     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    add r1, r1, #3      @soma op code de store bias
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    @bl enable                       @enable na instrucao
    @bl espera_done                  @agurda done para retorno
    bx lr

.global str_beta
.type str_beta, %function
str_beta:
    @r0 deve ser o hps_virtual
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    lsl r2, r2, #14     @dado lido no campo de dado 
    orr r1, r1, r2      @soma todos os bits em um ergistrador
    add r1, r1, #4      @soma op code de store beta
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    @ bl enable                       @enable na instrucao
    @ bl espera_done                  @agurda done para retorno
    @ bl clear_operation
    bx lr

.global str_wadress
.type str_wadress, %function
str_wadress:
    @r0 deve ser o hps_virtual
    lsl r1, r1, #3      @r1 agr tem o endereço no campo correto 
    add r1, r1, #1      @soma op code de store weigth adress
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    @ bl enable                       @enable na instrucao
    bx lr                           @não tem done, então só volta direto...

.global str_weight
.type str_weight, %function
str_weight:
    @r0 deve ser o hps_virtual
    lsl r1, r1, #3      @r1 agr tem o dado no campo correto 
    add r1, r1, #2      @soma op code de store weigth
    str r1, [r0, #PIO_INSTRUCTION]  @guarda instrucao no pio
    @ bl enable                       @enable na instrucao
    @ bl espera_done                  @agurda done para retorno
    @ bl clear_operation              
    bx lr

@ ==================== FLAGS ==================== 

.global espera_done
.type espera_done, %function
espera_done:
    push {lr}

espera:
    @tratamento da flag de erro aqui?
    ldr r1, [r0, #PIO_FLAG_DONE]    @guarda o sinal de done em r1
    cmp r1, #0                      @compara done com 0
    beq espera                      @se é zero, lê done de novo, até ser 1

    pop {pc}                           @retorna

.global get_flag_done
.type get_flag_done, %function
get_flag_done:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_DONE]
    bx lr

.global get_flag_busy
.type get_flag_busy, %function
get_flag_busy:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_BUSY]
    bx lr

.global get_flag_error
.type get_flag_error, %function
get_flag_error:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_ERROR]
    bx lr
