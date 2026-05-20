#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "colors.h"
#include "api.h"

#include <string.h>


int main() {
    bool done, busy, error;
    int inst;
    int a;
    int result;
    int loop=1;
    printf("\n============================\nOlá! Iniciando Coprocessador\n============================\n");
    void* hps_virtual = mapear();

    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }
    printf("\nMemória mapeada no endereço: %p\n", hps_virtual);

    printf("\n============================\n Iniciando Memória\n============================\n");
    printf("\nCarregando Bias na memória do Coprocessador...");
    store_bias(hps_virtual);//nunca vai dar erro pq o codamos bem :)
    printf("\nCarregando Beta na memória do Coprocessador...");
    store_beta(hps_virtual);

    printf("\nCarregando Pesos na memória do Coprocessador...");
    store_pesos(hps_virtual);


    printf("\nPré carregando imagem padrão 4...\n");
    store_image(hps_virtual);


    
        printf("\n%s[1] realizar a inferência da imagem definida%s\n", YELLOW, RESET);
        printf("\n%s[2] trocar a imagem de inferência%s\n", BLUE, RESET);
        printf("\n%s[3] para sair%s\n", CYAN, RESET);

    while(loop==1) {
        printf("\n%s=>%s ", YELLOW, RESET);
        scanf("%d", &inst);
        if(inst == 1){
            printf("\n========================================================\n");
            printf("\n[1] realizar a inferência da imagem definida\n");
            printf("\n[2] trocar a imagem de inferência\n");
            printf("\n[3] Teste de estabilidade inferencias\n");
            
            printf("\n[4] para sair\n");

            iniciar(hps_virtual);
            result = get_resultado(hps_virtual);
            printf("\nResultado inferência:%d\n", result);

            done = get_flag_done(hps_virtual); 
            printf("\nFlag de done: %d \n", done);

            busy = get_flag_busy(hps_virtual); 
            printf("Flag de busy: %d \n", busy);
            
            error = get_flag_error(hps_virtual);    
            printf("Flag de erro: %d \n", error);

            printf("\n========================================================\n");

        }else if (inst ==2){
            printf("\nFunção não implementada para o Marco 2\n");
        }else if (inst==3){
            int esperado;
            printf("\nResultado esperado:\n");

            scanf("%d", &esperado);

            int num, acertos=0;
            printf("\nDigite o número de inferencias a fazer:\n");
            scanf("%d", &num);
            int i;
            for (i = 0; i < num; i++)
            {
                
                iniciar(hps_virtual);
                result = get_resultado(hps_virtual);
                printf("\nResultado da %d inferência:%d\n", i+1 , result);
                if (result == esperado){
                    acertos++;
                    }
            }
            printf("\nAcertou %d de %d\n", acertos, num);
            float taxa = (acertos/num)*100;
            printf("\nTaxa de acerto: %f porcento\n", taxa);

        }
        else{printf("\nFinalizando o programa\n"); loop=0;}
    }

   
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}
