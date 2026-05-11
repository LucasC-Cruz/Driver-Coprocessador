#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Declaração das funções externas definidas no seu arquivo .s
extern void* mapear();

extern void status(void* hps_virtual);

extern void iniciar(void* hps_virtual);

extern void enable(void* hps_virtual, bool en);
extern void pulso_enable(void* hps_virtual);

extern void fechar(void* hps_virtual);

extern int resultado(void* hps_virtual);
extern bool flag_busy(void* hps_virtual);
extern bool flag_done(void* hps_virtual);
extern bool flag_error(void* hps_virtual);

extern bool reset(void* hps_virtual, bool rst);
extern bool clear_operation(void* hps_virtual, bool clr);


int main() {

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");

    void* hps_virtual = mapear();
        printf(" %p ", hps_virtual);

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }

    printf("Memória mapeada no endereço: %p\n", hps_virtual);


    reset(hps_virtual, 1);
    printf("Resetando coprocessador...\n");
    reset(hps_virtual, 0);

    clear_operation(hps_virtual, 1);
    printf("Resetando coprocessador...\n");
    clear_operation(hps_virtual, 0);

    
    
    printf("Enviando instrução para inciar inferencia...\n");
    iniciar(hps_virtual);

    // pulso_enable(hps_virtual);
    // /**/
    enable(hps_virtual, 1);
    printf("Levando Enable a 1...\n");
    enable(hps_virtual, 0);
    printf("Levando enable a 0...\n");

    int predicao = resultado(hps_virtual);
    printf("\nResultado da inferencia: %d \n", predicao);

    bool done = flag_done(hps_virtual); 
    printf("\nFlag de done: %d \n", done);

    bool busy = flag_busy(hps_virtual); 
    printf("\nFlag de busy: %d \n", busy);
    
    bool error = flag_error(hps_virtual); 
    printf("\nFlag de erro: %d \n", error);

    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");


    return 0;
}