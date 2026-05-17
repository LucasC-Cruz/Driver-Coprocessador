#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "colors.h"

// Declaração das funções externas definidas no seu arquivo .s
extern void* mapear();

extern void status(void* hps_virtual);

extern void iniciar(void* hps_virtual);

//extern void enable(void* hps_virtual, bool en);
extern void enable(void* hps_virtual);

extern void fechar(void* hps_virtual);

extern int get_resultado(void* hps_virtual);

extern bool get_flag_busy(void* hps_virtual);
extern bool get_flag_done(void* hps_virtual);
extern bool get_flag_error(void* hps_virtual);

extern void reset(void* hps_virtual);
extern void clear_operation(void* hps_virtual);

extern void instrucao(void* hps_virtual, int inst);

extern void store_image(void* hps_virtual);

extern void str_img(void* hps_virtual, int e, int d);

extern int confirmar(void* hps_virtual);

extern void print_reg(int reg){
    unsigned int mask = (unsigned int)reg;
    printf("A instrução construida é: ");
    for (int i = 31; i >= 0; i--)
    {
        printf("%d", (mask>>i) & 1);
    }
    printf("\n");

}

int main() {

    printf("\n============================\nOlá! Iniciando Coprocessador\n============================");
    
    void* hps_virtual = mapear();
    printf(" %p ", hps_virtual);
    
    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }
    bool done, busy, error;

    reset(hps_virtual);
    printf("Resetando coprocessador...\n");

    clear_operation(hps_virtual);
    printf("Resetando coprocessador...\n");

    done = get_flag_done(hps_virtual); 
    printf("\nFlag de done: %d \n", done);
    
    busy = get_flag_busy(hps_virtual); 
    printf("Flag de busy: %d \n", busy);
    
    error = get_flag_error(hps_virtual);    
    printf("Flag de erro: %d \n", error);

    printf("Memória mapeada no endereço: %p\n", hps_virtual);

    printf("Guardando Imagem...\n");

    store_image(hps_virtual);

    printf("Imagem guardada!\n");

    sleep(1);

    printf("Iniciando inferencia...\n");
    
    iniciar(hps_virtual);
    
    sleep(1);

    printf("Ativando enable...\n");

    enable(hps_virtual);

    sleep(1);
    
    
    int predicao = get_resultado(hps_virtual);
    printf("\n%sResultado da inferencia:%s %d \n", GREEN, RESET, predicao);
    
    done = get_flag_done(hps_virtual); 
    printf("\nFlag de done: %d \n", done);
    
    busy = get_flag_busy(hps_virtual); 
    printf("Flag de busy: %d \n", busy);
    
    error = get_flag_error(hps_virtual);    
    printf("Flag de erro: %d \n", error);
    
    printf("\nMapeamento encerrado.\n");

    reset(hps_virtual);
    printf("Resetando coprocessador...\n");

    clear_operation(hps_virtual);
    printf("Resetando coprocessador...\n");
    
    fechar(hps_virtual);

    return 0;
}

