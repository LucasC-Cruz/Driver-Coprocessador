.equ SYSCALL_OPEN,      5
.equ SYSCALL_CLOSE,     6
.equ SYSCALL_MMAP2,     192
.equ SYSCALL_MUNMAP,    91

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

@ .global reset
@ .type reset, %function
@ reset:
@     @r0 deve ser o hps_virtual
@     @r1 deve ser bool
@     str r1, [r0, #PIO_RESET_COP]
@     bx lr

.global reset
.type reset, %function
reset:
    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    bx lr

@ .global clear_operation
@ .type clear_operation, %function
@ clear_operation:
@     @r0 deve ser o hps_virtual
@     @r1 deve ser bool
@     str r1, [r0, #PIO_CLR_OP]
@     bx lr

.global clear_operation
.type clear_operation, %function
clear_operation:
    mov r1, #1
    str r1, [r0, #PIO_CLR_OP]
    mov r1, #0
    str r1, [r0, #PIO_CLR_OP]
    bx lr

.global instrucao
.type instrucao, %function
instrucao:
    @r0 deve ser o hps_virtual
    @r1 deve ser o opcode
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

.global resultado
.type resultado, %function
resultado:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_RESULTADO]
    bx lr

.global store
.type store, %function
store:
    @r0 deve ser o hps_virtual  
    @r1 endereco
    @r2 dado
    str r1, [r0, #PIO_INSTRUCTION]
    bx lr

.global flag_done
.type flag_done, %function
flag_done:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_DONE]
    bx lr

.global flag_busy
.type flag_busy, %function
flag_busy:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_BUSY]
    bx lr

.global flag_error
.type flag_error, %function
flag_error:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_FLAG_ERROR]
    bx lr

@ .global enable
@ .type enable, %function
@ enable:
@     @r0 deve ser o hps_virtual
@     str r1, [r0, #PIO_ENABLE]
@     bx lr

.global pulso_enable
.type pulso_enable, %function
pulso_enable:
    push {r1, lr}
    @r0 deve ser o hps_virtual
    mov r1, #1
    str r1, [r0, #PIO_ENABLE]
    mov r1, #0
    str r1, [r0, #PIO_ENABLE]
    pop {r1, pc}

.global fechar
.type fechar, %function
fechar:
    push {r7, lr}
    mov r1, #PAGINA
    mov r7, #SYSCALL_MUNMAP
    svc 0
    pop {r7, pc}
