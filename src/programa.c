#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "colors.h"

// mapeamento
extern void* mapear();
extern void fechar(void* hps_virtual);

//flags
extern bool get_flag_busy(void* hps_virtual);
extern bool get_flag_done(void* hps_virtual);
extern bool get_flag_error(void* hps_virtual);
extern int  get_resultado(void* hps_virtual);

// ins basicas
extern void instrucao(void* hps_virtual, int inst);
extern void iniciar(void* hps_virtual);
extern void status(void* hps_virtual);

//sinais de controle
extern void enable(void* hps_virtual);
extern void reset(void* hps_virtual);
extern void clear_operation(void* hps_virtual);

int main() {

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");

    void* hps_virtual = mapear();
        printf(" %p ", hps_virtual);

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }

    printf("Memória mapeada no endereço: %p\n", hps_virtual);

    int inst;
    printf("\n%s=>%s ", YELLOW, RESET);

    bool done, busy, error;

    while(scanf("%d", &inst) == 1 && inst != 67) {
        reset(hps_virtual);
        printf("Resetando coprocessador...\n");

        clear_operation(hps_virtual);
        printf("Resetando coprocessador...\n");

        instrucao(hps_virtual, inst);
        enable(hps_virtual);

        int predicao = get_resultado(hps_virtual);
        printf("\n%sResultado da inferencia:%s %d \n", GREEN, RESET, predicao);

        done = get_flag_done(hps_virtual); 
        printf("\nFlag de done: %d \n", done);

        busy = get_flag_busy(hps_virtual); 
        printf("Flag de busy: %d \n", busy);
        
        error = get_flag_error(hps_virtual);    
        printf("Flag de erro: %d \n", error);

        printf("\n%s=>%s ", YELLOW, RESET);
    }
    
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}