#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "colors.h"
#include "api.h"
#include "stb_image.h"
#include "img2bin.h"
#include "vga.h"

int ler_binario_para_decimal(char *msg, int n) {
    uint32_t resultado = 0;
    int contador_bits = 0;
    int ch;

    printf("\n%s\n", msg);

    // Lê os caracteres até processar n bits ou encontrar o Fim do Arquivo (EOF)
    while (contador_bits < n && (ch = getchar()) != EOF) {
        
        if (ch=='\n') break;

        // Se for um espaço em branco (' ', '\n', '\t'), apenas ignora e continua
        if (isspace(ch)) {
            continue;
        }

        // Se for 0 ou 1, processa o bit
        if (ch == '0' || ch == '1') {
            // Desloca o resultado atual 1 casa para a esquerda (multiplica por 2)
            // e adiciona o novo bit convertido de caractere para número inteiro
            resultado = (resultado << 1) | (ch - '0');
            contador_bits++;
        } else {
            // Se o usuário digitar algo diferente de 0, 1 ou espaço (como uma letra),
            // interrompe a leitura
            printf("\nCaractere diferente de 0 ou 1. Encerrando leitura.\n");
            break;
        }
    }

    return (int)resultado;
}

void imprimir_decimal_para_binario(long int valor) {
    // Começamos do bit 31 (mais à esquerda) e vamos até o bit 0 (mais à direita)
    for (int i = 31; i >= 0; i--) {
        
        // Move o bit de interesse para a primeira posição e usa '& 1' para isolá-lo
        int bit = (valor >> i) & 1;
        
        printf("%u", bit);
        
        // Adiciona um espaço sempre que o índice for múltiplo de 4.
        // A condição 'i != 0' evita colocar um espaço inútil logo no final.
        if (i % 4 == 0 && i != 0) {
            printf(" ");
        }
    }
    printf("\n"); // Quebra de linha final para deixar o terminal organizado
}

int main() {
    bool done, busy, error;
    int inst;
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
                //teste para exibir a imagem da inferencia
                //enviar_imagem exibe a imagem no binario "image.bin"
                //necessário outra função para exibir no momento em que está desenhando (outro modo)
                enviar_imagem(hps_virtual);
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
            {
                char filename[256];
                printf("\nDigite o nome do arquivo de imagem:\n");
                scanf("%s", filename);
                if (png2bin(filename) == 0) {
                    store_image(hps_virtual);
                    printf("Nova imagem foi reenviada\n");
                } else {
                    printf("Erro ao reenviar imagem\n Confira o nome do arquivo digitado.");
                }
                break;
            }

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
                int custom_inst = ler_binario_para_decimal("Digite a instrução personalizada (em binario)", 32);

                instruction(hps_virtual, custom_inst);
                enable(hps_virtual);

                printf("\nInstrução personalizada enviada\n");
                break;
            }

            //Enviar pixel
            
            case 9:
            {
                int endereco  = ler_binario_para_decimal("Digite o endereço para o pixel (em binario)", 10);
                int pixel = ler_binario_para_decimal("Digite o valor do pixel (em decimal): (em binario)", 8);

                str_img(hps_virtual, endereco, pixel);
                enable(hps_virtual);

                printf("\nPixel enviado\n");
                break;
            }

            //Enviar peso
            case 8:
            {
                int endereco = ler_binario_para_decimal("Digite o endereço para o peso (em binario)", 17);
                int peso = ler_binario_para_decimal("Digite o valor do peso (em binario)", 16);

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
                int endereco = ler_binario_para_decimal("Digite o endereço para o bias (em binario)", 7);
                int bias = ler_binario_para_decimal("Digite o valor do bias (em binario)", 16);

                str_bias(hps_virtual, endereco, bias);
                enable(hps_virtual);

                printf("\nBias enviado\n");
                break;
            }

            //Enviar beta
            case 11:
            {
                int endereco = ler_binario_para_decimal("Digite o endereço para o beta (em binario)", 11);
                int beta = ler_binario_para_decimal("Digite o valor do beta (em binario)", 16);

                str_beta(hps_virtual, endereco, beta);
                enable(hps_virtual);

                printf("\nBeta enviado\n");
                break;
            }

            //Confirmar operação enviada
            case 12:
            {
                long int ope = confirmar(hps_virtual);
                printf("\nOperação enviada: \n");
                imprimir_decimal_para_binario(ope);
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
            {
                printf("\nOpção inválida. Tente novamente.\n");
                break;
            }

        }

    } while (loop==1);
    
    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");
    
    return 0;
}
