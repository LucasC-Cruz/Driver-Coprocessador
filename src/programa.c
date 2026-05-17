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

extern int store_image(void* hps_virtual);
extern int store_bias(void* hps_virtual);
extern int store_beta(void* hps_virtual);
extern int store_pesos(void* hps_virtual);



int main() {
    bool done, busy, error;
    int inst;
    int a;

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");
    void* hps_virtual = mapear();
        printf(" %p ", hps_virtual);

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }
    printf("Memória mapeada no endereço: %p\n", hps_virtual);
    printf("\n%s=>%s ", YELLOW, RESET);

    printf("\n============================\n Iniciando Memória\n============================");
    printf("%sCarregando Bias na memória do Coprocessador%s", RESET, YELLOW);
    a = store_bias(hps_virtual);
        if(a){printf("Erro carregamento Bias!");}

    printf("%sCarregando Beta na memória do Coprocessador%s", RESET, BLUE);
    a = store_beta(hps_virtual);
        if(a){printf("Erro carregamento Beta!");}

    printf("%sCarregando Pesos na memória do Coprocessador%s", RESET, CYAN);
    a = store_pesos(hps_virtual);
        if(a){printf("Erro carregamento Pesos!");}

    printf("%sPré carregando imagem padrão 4%s", RESET, WHITE);
    a = store_image(hps_virtual);
        if(a){printf("Erro carregamento Imagem!");}
    
    


    while(scanf("%d", &inst) == 1 && inst != 67) {
        printf("Digite 1 se quiser realizar a inferência da imagem definida");
        prinft("Digite 2 para trocar a imagem para inferência");
        printf("Digite 67 para sair");

        reset(hps_virtual);
        printf("Resetando coprocessador...\n");
        if(inst){
            iniciar(hps_virtual);
            get_resultado(hps_virtual);

            done = get_flag_done(hps_virtual); 
            printf("\nFlag de done: %d \n", done);

            busy = get_flag_busy(hps_virtual); 
            printf("Flag de busy: %d \n", busy);
            
            error = get_flag_error(hps_virtual);    
            printf("Flag de erro: %d \n", error);

            printf("\n%s=>%s ", YELLOW, RESET);
        }else if(inst == 2){printf("Função não implementada para o Marco 2"); inst = 67;}
    }

   
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}