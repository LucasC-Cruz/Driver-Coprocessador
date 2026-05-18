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

extern void store_image(void* hps_virtual);
extern void store_bias(void* hps_virtual);
extern void store_beta(void* hps_virtual);
extern void store_pesos(void* hps_virtual);



int main() {
    bool done, busy, error;
    int inst;
    int a;
    int result;

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");
    void* hps_virtual = mapear();

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }
    printf("Memória mapeada no endereço: %p\n", hps_virtual);
    printf("\n%s=>%s ", RESET, YELLOW);

    printf("\n============================\n Iniciando Memória\n============================\n");
    printf("%sCarregando Bias na memória do Coprocessador%s", YELLOW, RESET);
    store_bias(hps_virtual);
    printf("\nTerminei de iniciar bias\n");

    printf("%sCarregando Beta na memória do Coprocessador%s", BLUE, RESET);
    store_beta(hps_virtual);
    printf("\nTerminei de iniciar beta\n");

    printf("%sCarregando Pesos na memória do Coprocessador%s", CYAN, RESET);
    store_pesos(hps_virtual);
    printf("\nTerminei de iniciar pesos\n");
        

    printf("%sPré carregando imagem padrão 4%s",  WHITE, RESET);
    store_image(hps_virtual);
    printf("\nTerminei de iniciar imagem\n");

    
        printf("\nDigite 1 se quiser realizar a inferência da imagem definida\n");
        printf("\nDigite 2 para trocar a imagem para inferência\n");
        printf("\nDigite 67 para sair\n");
        reset(hps_virtual);
        iniciar(hps_virtual);

        printf("Resetando esta buceta\n");

    while( (inst != 67) && (scanf("%d", &inst) == 1)) {
        printf("\nDigite 1 se quiser realizar a inferência da imagem definida\n");
        printf("\nDigite 2 para trocar a imagem para inferência\n");
        printf("\nDigite 67 para sair\n");

        if(inst !=2){
            iniciar(hps_virtual);
            result = get_resultado(hps_virtual);
            printf("\nResultado inferência: %d \n", result);

            done = get_flag_done(hps_virtual); 
            printf("\nFlag de done: %d \n", done);

            busy = get_flag_busy(hps_virtual); 
            printf("Flag de busy: %d \n", busy);
            
            error = get_flag_error(hps_virtual);    
            printf("Flag de erro: %d \n", error);

            printf("\n%s=>%s ", YELLOW, RESET);
        }else {
            printf("Função não implementada para o Marco 2");
            inst = 67;
        }
    }

   
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}
