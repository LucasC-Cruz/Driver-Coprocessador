.equ SYSCALL_EXIT,     1
.equ SYSCALL_OPEN,     5
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_MMAP2,    192
.equ SYSCALL_MUNMAP,   91

.equ PAGINA_TAMANHO,   0x5000
.equ LWHPS2FPGA_BASE,  0xFF200    @ Base / 4096 (mmap2 usa offset em páginas)
.equ PROT_READ_WRITE,  0x3
.equ MAP_SHARED,       0x1

@ Offsets baseados no seu hps_0.h (exemplo comum)
.equ PIO_LED_BASE,     0x00       @ Ajuste conforme seu hps_0.h
.equ PIO_SW_BASE,      0x40       @ Ajuste conforme seu hps_0.h

.section .data
dev_mem:    .asciz "/dev/mem"
err_msg:    .asciz "Erro ao abrir /dev/mem ou mmap\n"

.section .text
.global _start

_start:
    @ --- open("/dev/mem", O_RDWR | O_SYNC) ---
    ldr r0, =dev_mem
    ldr r1, =0x101002      @ O_RDWR | O_SYNC
    mov r7, #SYSCALL_OPEN
    svc 0
    cmp r0, #0
    blt error
    mov r4, r0             @ r4 = fd

    @ --- mmap2(NULL, SPAN, READ|WRITE, SHARED, fd, BASE>>12) ---
    mov r0, #0             @ addr = NULL
    ldr r1, =PAGINA_TAMANHO
    mov r2, #PROT_READ_WRITE
    mov r3, #MAP_SHARED
    @ r4 já tem o fd
    ldr r5, =LWHPS2FPGA_BASE
    mov r7, #SYSCALL_MMAP2
    svc 0
    cmp r0, #-1
    beq error
    mov r5, r0             @ r5 = Endereço Virtual Base

    @ --- Loop Principal ---
    @ r6 = Ponteiro LED (r5 + PIO_LED_BASE)
    @ r8 = Ponteiro SW  (r5 + PIO_SW_BASE)
    add r6, r5, #PIO_LED_BASE
    add r8, r5, #PIO_SW_BASE

loop_infinito:
    ldr r1, [r8]           @ r1 = *SW_ptr (leitura dos switches)
    mov r2, #0             @ r2 = count = 0

contar_bits:
    cmp r1, #0
    beq atualizar_leds
    tst r1, #1             @ Testa o bit 0
    addne r2, r2, #1       @ Se não zero, count++
    lsr r1, r1, #1         @ r1 >>= 1
    b contar_bits

atualizar_leds:
    @ Lógica: (1 << count) - 1
    mov r3, #1
    lsl r3, r3, r2         @ r3 = 1 << count
    sub r3, r3, #1         @ r3 = (1 << count) - 1
    str r3, [r6]           @ *LEDR_ptr = r3

    b loop_infinito

error:
    mov r0, #1             @ stderr
    ldr r1, =err_msg
    mov r2, #32
    mov r7, #4             @ write
    svc 0
    mov r7, #SYSCALL_EXIT
    svc 0
