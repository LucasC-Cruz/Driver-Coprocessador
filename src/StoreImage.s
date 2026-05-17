.extern str_img

.equ SYSCALL_EXIT,     1
.equ SYSCALL_READ,     3
.equ SYSCALL_WRITE,    4
.equ SYSCALL_OPEN,     5
.equ SYSCALL_CLOSE,    6
.equ READ_ONLY,        0

.section .bss
.align 2
    buffer_img:  .skip 784     @abre 784 bytes de espaço

.section .data
    buffer_filename: .asciz "imagem_4.bin"

@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.section .text 
.global store_image
    .type store_image, %function
    store_image:
        push {r4-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        mov r7, #SYSCALL_OPEN       
        ldr r0, =buffer_filename
        mov r1, #READ_ONLY          
        svc #0    
        mov r6, r0                   @ guarda fd em r6 para fechar depois

        mov r7, #SYSCALL_READ
        ldr r1, =buffer_img     
        mov r2, #784              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r3, =buffer_img         @ponteiro do buffer
        mov r5, #0                  @ contador

        mov r0, r8  @hps virtual, necessario para a chamada

    store_loop:
        ldrb r2, [r3], #1   @lendo o valor do buffer para r1, e incrementa ponteiro

        @r0 hps
        @r2 já tem o dado
        @r1 deve conter o endereço, que está e atualizado em r5
        mov r1, r5
        bl str_img @guarda no pio ins, da enable e espera done

        add r5, #1
        cmp r5, #784
        bne store_loop
    
    @fechar
        mov r7, #SYSCALL_CLOSE
        mov r0, r6
        svc 0

        pop {r4-r8, pc}
