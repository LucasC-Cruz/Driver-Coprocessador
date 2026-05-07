#include <stdio.h>
#include <stdint.h>

// Declaração das funções externas definidas no seu arquivo .s
extern void* mapear();
extern void iniciar(void* hps_virtual);
extern void status(void* hps_virtual);
extern void enable(void* hps_virtual);
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
    
    printf("Verificando status...\n");
    status(hps_virtual);

    printf("Iniciando inferencia...\n");
    iniciar(hps_virtual);

    printf("Habilitando (Enable)...\n");
    enable(hps_virtual);
    
    int a = resultado(hps_virtual);
    printf("%d", a);

    fechar(hps_virtual);
    printf("Mapeamento encerrado.\n");


    return 0;
}