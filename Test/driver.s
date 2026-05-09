.equ SYSCALL_OPEN,     5
.equ SYSCALL_CLOSE,    6
.equ SYSCALL_MMAP2,    192
.equ SYSCALL_MUNMAP,   91


.equ PROT_READ_WRITE,  0x3
.equ MAP_SHARED,       0x01
.equ PAGINA,   0x5000
.equ LWHPS2FPGA_BASE,  0xFF200
.equ PIO_INS,   0x20 
.equ PIO_EN,   0x00 @acho que esse endereço está errado para o pio do enable
.equ PIO_OUT, 0x30

.section .data
dev_mem: .asciz "/dev/mem"

.section .text
.global mapear
.type mapear, %function
mapear:  
    ldr r0, =dev_mem
    ldr r1, =0x101002           @O_RDWR|O_SYNC
    mov r7, #SYSCALL_OPEN
    svc 0                       @r0 contera fd
                                @colocar tratamento de erro aqui

    mov r4, r0                  @fd da abertura para mapeamento
    mov r0, #0                  @endereço para o mapeamento
    ldr r1, =PAGINA
    mov r2, PROT_READ_WRITE     @PROT_READ | PROT_WRITE
    mov r3, MAP_SHARED          @MAP_SHARED
    ldr r5, =LWHPS2FPGA_BASE    
    mov r7, #SYSCALL_MMAP2
    svc 0
    bx lr
  

  @recebe no r0 o valor passado pelo C
.global iniciar
.type iniciar, %function
iniciar:
    @r0 deve ser o hps_virtual
    mov r1, #5                  @#5 é opcode de start
    str r1, [r0, #PIO_INS]
    bx lr

.global status
.type status, %function
status:
    @r0 deve ser o hps_virtual
    mov r1, #6                  @#6 é opcode de status
    str r1, [r0, #PIO_INS]
    bx lr

@funcao para retorna pio out(valor inferiddo e flags)
.global resultado
.type resultado, %function
resultado:
    @r0 deve ser o hps_virtual
    ldr r0, [r0, #PIO_OUT]
    bx lr


.global enable
.type enable, %function
enable:
    @r0 deve ser o hps_virtual
    mov r1, #1
    str r1, [r0, #PIO_EN] @o hps sempre pega o bit menos
    @significativo 
    bx lr


.global enable
.type disable, %function
enable:
    @r0 deve ser o hps_virtual
    mov r1, #0
    str r1, [r0, #PIO_EN]
    bx lr


.global fechar
.type fechar, %function
fechar:
    push {r7, lr}
    mov r1, #PAGINA
    mov r7, #SYSCALL_MUNMAP
    svc 0
    pop {r7, pc}
