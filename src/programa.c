#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "colors.h"

// Declaração das funções externas definidas no seu arquivo .s
extern void* mapear();

extern void status(void* hps_virtual);

extern void iniciar(void* hps_virtual);

//extern void enable(void* hps_virtual, bool en);
extern void pulso_enable(void* hps_virtual);

extern void fechar(void* hps_virtual);

extern int resultado(void* hps_virtual);

extern int store_image(void* hps_virtual, int endereco, int dado);

extern bool flag_busy(void* hps_virtual);
extern bool flag_done(void* hps_virtual);
extern bool flag_error(void* hps_virtual);

extern void reset(void* hps_virtual);
extern void clear_operation(void* hps_virtual);
extern void instrucao(void* hps_virtual, int inst);



int main() {

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");

    void* hps_virtual = mapear();
        printf(" %p ", hps_virtual);

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }

    printf("Memória mapeada no endereço: %p\n", hps_virtual);

    // int inst;
    // printf("\n%s=>%s ", YELLOW, RESET);
    // while(scanf("%d", &inst) == 1 && inst != 67) {
    //     reset(hps_virtual);
    //     printf("Resetando coprocessador...\n");

    //     clear_operation(hps_virtual);
    //     printf("Resetando coprocessador...\n");

    //     instrucao(hps_virtual, inst);
    //     pulso_enable(hps_virtual);

    //     int predicao = resultado(hps_virtual);
    //     printf("\n%sResultado da inferencia:%s %d \n", GREEN, RESET, predicao);

    //     bool done = flag_done(hps_virtual); 
    //     printf("\nFlag de done: %d \n", done);

    //     bool busy = flag_busy(hps_virtual); 
    //     printf("Flag de busy: %d \n", busy);
        
    //     bool error = flag_error(hps_virtual); 
    //     printf("Flag de erro: %d \n", error);

    //     printf("\n%s=>%s ", YELLOW, RESET);

    // }

        reset(hps_virtual);
        printf("Resetando coprocessador...\n");

        clear_operation(hps_virtual);
        printf("Resetando coprocessador...\n");

        int instr = store_image(hps_virtual, 1, 1);
        pulso_enable(hps_virtual);
        printf("\n%sINSTRUÇÃO:%s %d \n", GREEN, RESET, instr);


        int predicao = resultado(hps_virtual);
        printf("\n%sResultado da inferencia:%s %d \n", GREEN, RESET, predicao);

        bool done = flag_done(hps_virtual); 
        printf("\nFlag de done: %d \n", done);

        bool busy = flag_busy(hps_virtual); 
        printf("Flag de busy: %d \n", busy);
        
        bool error = flag_error(hps_virtual); 
        printf("Flag de erro: %d \n", error);

        printf("\n%s=>%s ", YELLOW, RESET);

    
            fechar(hps_virtual);
            printf("\nMapeamento encerrado.\n");
    
    return 0;
}
