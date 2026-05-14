.extern enable

.equ SYSCALL_EXIT,     1
.equ SYSCALL_READ,     3
.equ SYSCALL_WRITE,    4
.equ SYSCALL_OPEN,     5
.equ SYSCALL_CLOSE,    6
.equ READ_ONLY,        0

.equ PIO_INSTRUCTION,   0x00 
.equ PIO_ENABLE,        0x10 

.section .bss
.align 2
    buffer_img:  .skip 784     @abre 784 bytes de espaço

.section .data
    buffer_filename: .asciz "img.bin"

@ r0 tem hps virtual
@ r1 offset pio da instrucao 
.section .text 
    .type store_image, %function
    store_image:
        push {r4-r8, lr}
        mov r8, r0              @hps virtual

        @abertura do arquivo binario
        mov r7, #SYSCALL_OPEN       
        ldr r0, =buffer_filename
        mov r1, #READ_ONLY          
        svc #0                      @ retorna fd em r0
        mov r6, r0

        mov r7, #SYSCALL_READ
        ldr r1, =buffer_img     
        mov r2, #784              @ Quantos bytes vai ler
        svc #0                      @ Buffer foi preenchido com dados do arquivo

        ldr r2, =buffer_img
        mov r5, #0

    store_loop:
        @hps, necessario para envio da instrução e chamada de get_flag_done 
        mov r0, r8
        
        ldrb r1, [r2], #1   @lendo o valor do buffer para r1, e incrementa ponteiro
        @  add r2, r2, #1  @incrementando ponteiro do buffer por um byte (8bits)

        @montando instrução
        mov r4, r5      @ r5 tem contador, que é igual ao endereço em que será guardado o dado atual
        
        lsl r4, #3      @r4 agr tem o endereço no campo correto 
        lsl r1, #13     @dado lido no campo de dado    
        
        add r1, r1, r4

        @enviando instrução
        str r1, [r8, #PIO_INSTRUCTION]
        bl enable
    
        @confirmando done
        espera:
            bl get_flag_done
            cmp r0, #0
            beq espera

        add r5, #1
        cmp r5, #784
        bne store_loop
    
    fechar:
        mov r7, #SYSCALL_CLOSE
        mov r0, r6
        svc 0

        pop {r4-r8, pc}
