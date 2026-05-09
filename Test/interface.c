#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

// Declaração das funções externas definidas no seu arquivo .s
extern void* mapear();
extern void iniciar();
extern void status(void* hps_virtual);
extern void enable(void* hps_virtual);
extern void disable(void* hps_virtual);
extern void fechar(void* hps_virtual);
extern int resultado(void* hps_virtual);

// extern void babalu();

int main() {

    printf("Olá, iniciando coprocessador\n");

    void* hps_virtual = mapear();
     printf(" %p ", hps_virtual);
     
    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }

    printf("Memória mapeada no endereço: %p\n", hps_virtual);
    ;

    printf("Iniciando inferencia...\n");
    iniciar(hps_virtual);

    printf("Habilitando (Enable)...\n");
    enable(hps_virtual);

    sleep(3); //dorme por 2 segundos (tempo de fazer conta)
    
    printf("Desabilitando para mostrar resultado");
    disable(hps_virtual);

    sleep(3); //garante que o resultando seja escrito no pio

    volatile uint32_t a = resultado(hps_virtual);
    printf("%d", a);

    fechar(hps_virtual);
    printf("Mapeamento encerrado.\n");


    return 0;
}