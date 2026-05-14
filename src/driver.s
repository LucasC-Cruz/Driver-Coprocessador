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
    mov r1, #1
    str r1, [r0, #PIO_ENABLE]
    mov r1, #0
    str r1, [r0, #PIO_ENABLE]
    bx lr

.global reset
.type reset, %function
reset:
    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    bx lr

.global clear_operation
.type clear_operation, %function
clear_operation:
    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    bx lr

@ ==================== INSTRUÇÕES BÁSICAS ==================== 

.global instruction
.type instruction, %function
instruction:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

.global iniciar
.type iniciar, %function
iniciar:
    @r0 deve ser o hps_virtual
    mov r1, #5
    str r1, [r0, #PIO_INSTRUCTION]
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

.global resultado
.type resultado, %function
resultado:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_RESULTADO]
    bx lr

@ ==================== FLAGS ==================== 

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
