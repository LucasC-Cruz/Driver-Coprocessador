#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "colors.h"
#include "api.h"
#include <fcntl.h>
 
//aura + ego
char* trim(char* str) {
    // Remove do final
    int len = strlen(str); //strlen para no \0 adicionado pelo fgets
    while (len > 0 && (isspace((unsigned char) str[len - 1]))) {
        str[--len] = '\0';
    }
    
    // Remove do início
    int start = 0;
    while (isspace(str[start])) {
        start++;
    }
    
    // Move a string
    //move o ponteiro do inicio da string pra frente, :)
    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
    
    return str;
}
 
// Função para limpar o buffer do stdin
void limparBuffer() {
    int c;
    //pega e remove ao mesmo tempo com o getchar, se o caba removido for o \n 
    while ((c = getchar()) != '\n' && c != EOF);
}
 
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

    int num_inst = total_inst();
    
    reset(hps_virtual);
    iniciar(hps_virtual);
 
    do
    {
        printf("\n========================================================\n");
        printf("%s[1] Realizar a inferencia de imagem definida%s\n", YELLOW, RESET);
        printf("%s[2] Realizar a inferencia da imagem desenhada na tela%s\n", YELLOW, RESET);
        printf("%s[3] Benchmark%s\n", YELLOW, RESET);
        printf("%s[4] Outras Opções%s\n", YELLOW, RESET);
        printf("%s[0] para sair%s\n", YELLOW, RESET);
        printf("\n========================================================\n");
 
        printf("\n%s=>%s ", YELLOW, RESET);
        scanf("%d", &inst);
 
        switch (inst)
        {
            case 0:
            {
                printf("\nFinalizando o programa\n"); 
                loop=0;
                break;
            }
 
            // realizar a inferência da imagem definida
            // receber caminho da imagem
            //acho bom botar a opção de inferir novamente a mesma imagem sem ter que digitar o caminho todo
            case 1:
            {
                int loop2 = 1;
                char caminho[50];
                char* camin;   
                limparBuffer();
                while(loop2 == 1){
                    printf("Digite o caminho da imagem:");  
                    printf("Formato aceitavel");
                    printf("xxx/xxx/xxx.bin");
                    printf("[0] Cancelar.");
                    printf("\n%s=>%s ", YELLOW, RESET);
                    fgets(caminho, sizeof(caminho), stdin);
                    //ponteiro da string que será mandada para o driver
                    camin = trim(caminho);
                    if(*camin == '0' && strlen(camin) == 1) loop2=0;
                    store_image(hps_virtual, camin);

                    int esperado;
                    int result;
                    printf("\nResultado esperado:\n");
                    scanf("%d", &esperado);
                    result = get_resultado(hps_virtual);

                    printf("\nResultado da inferência:%d\n",result);
                    if (result == esperado){printf("Acertou!");}

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
                }
                printf("\n========================================================\n");
                break;
            }                    
 
            case 2:
            {
                printf("\nFunção não implementada ainda2\n");
                break;
            }
 
            // Benchmark
            case 3:
            {
                //            
                break;
            }
 
            // Outras opções
            case 4:
            {
                int loop1 = 1;
                int a;
                while(loop1 == 1){
 
                    printf("\n========================================================\n");
                    printf("%s[1]  Resetar o Coprocessador%s\n", RED, RESET);
                    printf("%s[2]  Clear operation%s\n", RED, RESET);
                    printf("%s[3]  Enviar operação NOP%s\n", GREEN, RESET);
                    printf("%s[4]  Enviar instrução personalizada%s\n", GREEN, RESET);
                    printf("%s[5]  Enviar pixel%s\n", GREEN, RESET);
                    printf("%s[6]  Enviar peso%s\n", GREEN, RESET);
                    printf("%s[7]  Enviar bias%s\n", GREEN, RESET);
                    printf("%s[8]  Enviar beta%s\n", GREEN, RESET);
                    printf("%s[9]  Confirmar operação enviada%s\n", GREEN, RESET);
                    printf("%s[10] Reenviar imagem prédefinida%s\n", CYAN, RESET);
                    printf("%s[11] Reenviar bias%s\n", CYAN, RESET);
                    printf("%s[12] Reenviar beta%s\n", CYAN, RESET);
                    printf("%s[13] Reenviar pesos%s\n", CYAN, RESET);
                    printf("%s[0]  Voltar%s\n", YELLOW, RESET);
                    printf("\n========================================================\n");
                    printf("\n%s=>%s ", YELLOW, RESET);
                    scanf("%d", &a);
 
                    switch(a)
                    {
                        case 0:
                        {
                            loop1 = 0;
                            break;
                        }
 
                        // Resetar o Coprocessador
                        case 1:
                        {
                            // reset(hps_virtual);
                            printf("\nCoprocessador resetado\n");
                            break;
                        }
 
                        // Clear operation
                        case 2:
                        {
                            clear_operation(hps_virtual);
                            enable(hps_virtual);
                            printf("\nOperação limpa\n");
                            break;
                        }
 
                        // Enviar operação NOP
                        case 3:
                        {
                            NO_OP(hps_virtual);
                            enable(hps_virtual);
                            printf("\nOperação NOP enviada\n");
                            break;
                        }
 
                        // Enviar instrução personalizada (antigo case 7)
                        case 4:
                        {
                            int custom_inst;
                            printf("\nDigite a instrução personalizada (em decimal):\n");
                            scanf("%d", &custom_inst);
                            instruction(hps_virtual, custom_inst);
                            enable(hps_virtual);
                            printf("\nInstrução personalizada enviada\n");
                            break;
                        }
 
                        // Enviar pixel (antigo case 9)
                        case 5:
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
 
                        // Enviar peso (antigo case 8)
                        case 6:
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
 
                        // Enviar bias (antigo case 10)
                        case 7:
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
 
                        // Enviar beta (antigo case 11)
                        case 8:
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
 
                        // Confirmar operação enviada (antigo case 12)
                        case 9:
                        {
                            long int ope = confirmar(hps_virtual);
                            printf("\nOperação enviada: %ld\n", ope);
                            break;
                        }
 
                        // Reenviar imagem prédefinida (antigo case 13)
                        case 10:
                        {
                            store_image(hps_virtual, "imagem_4.bin");
                            printf("\nImagem prédefinida reenviada\n");
                            break;
                        }
 
                        // Reenviar bias (antigo case 14)
                        case 11:
                        {
                            store_bias(hps_virtual);
                            printf("\nBias reenviado\n");
                            break;
                        }
 
                        // Reenviar beta (antigo case 15)
                        case 12:
                        {
                            store_beta(hps_virtual);
                            printf("\nBeta reenviado\n");
                            break;
                        }
 
                        // Reenviar pesos (antigo case 16)
                        case 13:
                        {
                            store_pesos(hps_virtual);
                            printf("\nPesos reenviados\n");
                            break;
                        }
 
                        default:
                        {
                            printf("\nOpção inválida. Tente novamente.\n");
                            break;
                        }
                    }
                }
                break;
            }
 
            default:
            {
                printf("\nOpção inválida. Tente novamente.\n");
                break;
            }
        }
    } while (loop==1);
    
    // fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}
