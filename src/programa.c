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

    zera_inst();

    printf("\nMemória mapeada no endereço: %p\n", hps_virtual);

    printf("\n============================\n Iniciando Memória\n============================\n");
    printf("\nCarregando Bias na memória do Coprocessador...");
    store_bias(hps_virtual);//nunca vai dar erro pq o codamos bem :)

    printf("\nCarregando Beta na memória do Coprocessador...");
    store_beta(hps_virtual);

    printf("\nCarregando Pesos na memória do Coprocessador...");
    store_pesos(hps_virtual);

    printf("\nPré carregando imagem padrão...\n");
    store_image(hps_virtual);

    int num_inst = total_inst();
    
    reset(hps_virtual);
    iniciar(hps_virtual);

    

    do
    {
        printf("\n========================================================\n");
        printf("%s[1] realizar a inferência da imagem prédefinida%s\n", YELLOW, RESET);
        printf("%s[2] trocar a imagem de inferência%s\n", YELLOW, RESET);
        printf("%s[3] Teste de estabilidade inferencias%s\n", YELLOW, RESET);


        printf("%s[4] Resetar o Coprocessador%s\n", RED, RESET);
        printf("%s[5] Clear operation%s\n", RED, RESET);
        printf("%s[6] Enviar operação NOP%s\n", GREEN, RESET);
        printf("%s[7] Enviar instrução personalizada%s\n", GREEN, RESET);
        printf("%s[9] Enviar pixel%s\n", GREEN, RESET);
        printf("%s[8] Enviar peso%s\n", GREEN, RESET);
        printf("%s[10] Enviar bias%s\n", GREEN, RESET);
        printf("%s[11] Enviar beta%s\n", GREEN, RESET);
        printf("%s[12] Confirmar operação enviada%s\n", GREEN, RESET);

        printf("%s[13] Reenviar imagem prédefinida\n%s", CYAN, RESET);
        printf("%s[14] Reenviar bias\n%s", CYAN, RESET);
        printf("%s[15] Reenviar beta\n%s", CYAN, RESET);
        printf("%s[16] Reenviar pesos\n%s", CYAN, RESET);

        printf("\n========================================================\n");
        printf("[0] para sair\n");

        printf("\n%s=>%s ", YELLOW, RESET);
        scanf("%d", &inst);

        switch (inst)
        {
            case 0:
            {
                printf("\nFinalizando o programa\n"); 
                loop=0;
            }
            //realizar a inferência da imagem definida
            case 1:
            {
                iniciar(hps_virtual);
                result = get_resultado(hps_virtual);
                printf("\nResultado inferência: %d\n", result);

                
                printf("\nNúmero de instruções de memória enviadas: %d\n", num_inst);

                //número de instruções de memória * 5 +  clocks primeira camada (32 * 18844) + clocks segunda camada 2*(18844) + 
                // clocks argmax 10 + clock de controle 2
                int totalClocks = (num_inst *5) + (32*18844) + (2*18844) + 10 + 2;
                printf("\nNúmero de clocks: %d\n", totalClocks);

                done = get_flag_done(hps_virtual); 
                printf("\nFlag de done: %d \n", done);

                busy = get_flag_busy(hps_virtual); 
                printf("Flag de busy: %d \n", busy);
                
                error = get_flag_error(hps_virtual);    
                printf("Flag de erro: %d \n", error);

                printf("\n========================================================\n");
                break;
            }

            //trocar a imagem de inferência
            case 2:
                printf("\nFunção não implementada para o Marco 2\n");
            break;

            //Teste de estabilidade inferencias
            case 3:
            {
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
                    if (result == esperado)
                    {
                        acertos++;
                    }
                }
                printf("\nAcertou %d de %d\n", acertos, num);
                float taxa = (acertos/num)*100;
                printf("\nTaxa de acerto: %f porcento\n", taxa);
                break;
            }
            
            //Resetar o Coprocessador
            case 4:
            {
                reset(hps_virtual);
                printf("\nCoprocessador resetado\n");
                break;
            }
            
            //Clear operation
            case 5:
            {
                clear_operation(hps_virtual);
                enable(hps_virtual);
                printf("\nOperação limpa\n");
                break;
            }

            //Enviar operação NOP
            case 6:
            {
                NO_OP(hps_virtual);
                enable(hps_virtual);
                printf("\nOperação NOP enviada\n");
                break;
            }

            //Enviar instrução personalizada
            case 7:
            {
                int custom_inst;
                printf("\nDigite a instrução personalizada (em decimal):\n");
                scanf("%d", &custom_inst);
                instruction(hps_virtual, custom_inst);
                enable(hps_virtual);
                printf("\nInstrução personalizada enviada\n");
                break;
            }

            //Enviar pixel
            case 9:
            {
                int endereco, pixel;
                printf("\nDigite o endereço para o pixel (em decimal):\n");
                scanf("%d", &endereco);
                printf("\nDigite o valor do pixel (em decimal):\n");
                scanf("%d", &pixel);
                str_img(hps_virtual, endereco, pixel);
                enable(hps_virtual);
                printf("\nPixel enviado\n");
                break;
            }

            //Enviar peso
            case 8:
            {
                int endereco, peso;
                printf("\nDigite o endereço para o peso (em decimal):\n");
                scanf("%d", &endereco);
                printf("\nDigite o valor do peso (em decimal):\n");
                scanf("%d", &peso);
                str_wadress(hps_virtual, endereco);
                enable(hps_virtual);
                str_weight(hps_virtual, 0, peso);
                enable(hps_virtual);
                printf("\nPeso enviado\n");
                break;
            }

            //Enviar bias
            case 10:
            {
                int endereco, bias;
                printf("\nDigite o endereço para o bias (em decimal):\n");
                scanf("%d", &endereco);
                printf("\nDigite o valor do bias (em decimal):\n");
                scanf("%d", &bias);
                str_bias(hps_virtual, endereco, bias);
                enable(hps_virtual);
                printf("\nBias enviado\n");
                break;
            }

            //Enviar beta
            case 11:
            {
                int endereco, beta;
                printf("\nDigite o endereço para o beta (em decimal):\n");
                scanf("%d", &endereco);
                printf("\nDigite o valor do beta (em decimal):\n");
                scanf("%d", &beta);
                str_beta(hps_virtual, endereco, beta);
                enable(hps_virtual);
                printf("\nBeta enviado\n");
                break;
            }

            //Confirmar operação enviada
            case 12:
            {
                long int ope = confirmar(hps_virtual);
                printf("\nOperação enviada: %ld\n", ope);
                break;
            }

            case 13:
            {
                store_image(hps_virtual);
                printf("\nImagem prédefinida reenviada\n");
                break;
            }

            case 14:
            {
                store_bias(hps_virtual);
                printf("\nBias reenviado\n");
                break;
            }

            case 15:
            {
                store_beta(hps_virtual);
                printf("\nBeta reenviado\n");
                break;
            }

            case 16:
            {
                store_pesos(hps_virtual);
                printf("\nPesos reenviados\n");
                break;
            }
            default :
                printf("\nOpção inválida. Tente novamente.\n");
                break;

        }

    } while (loop==1);
    
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}
