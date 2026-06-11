#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#include "colors.h"
#include "api.h"
#include "img2bin.h"
#include "vga.h"
#include "mouse.h"
#include "pio.h"

 
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
    
    char* caminho_img_bin = "assets/image.bin";
    char* caminho_desenho = "assets/desenho.bin";
    bool done, busy, error;
    int inst;
    int a; //usado para selecionar opção no segundo menu
    int loop=1;
    printf("\n============================\nOlá! Iniciando Coprocessador\n============================\n");
    void* hps_virtual = mapear();
    
    if ((intptr_t)hps_virtual == -1) {
        printf("Erro ao mapear memória");
        return 1;
    }
    vga_pio_ptr  = (volatile uint32_t *)(hps_virtual + (PIO_VGA_OFFSET));
    vga_done_ptr = (volatile uint32_t *)(hps_virtual + (PIO_VGA_DONE_OFFSET));

    zera_inst();

    printf("\nMemória mapeada no endereço: %p\n", hps_virtual);

    printf("\n============================\n Iniciando Memória\n============================\n");
    printf("\nCarregando Bias na memória do Coprocessador...");
    store_bias(hps_virtual);//nunca vai dar erro pq o codamos bem :)

    printf("\nCarregando Beta na memória do Coprocessador...");
    store_beta(hps_virtual);

    printf("\nCarregando Pesos na memória do Coprocessador...");
    store_pesos(hps_virtual);

    //talvez isto esteja quebrado agora na verdade
    int num_inst = total_inst();
    //possivelmente teremos que resetar ao longo dos fluxos de inferência
     
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
                limparBuffer();
                while(loop2 == 1){
                
                    reset(hps_virtual);
                    //resetando

                    printf("Digite o caminho da imagem:");  
                    printf("Formato aceitavel");
                    printf("xxx/xxx/xxx.png");
                    printf("[0] Cancelar.");
                    printf("\n%s=>%s ", YELLOW, RESET);
                    
                    char filename[256];
                    printf("\nDigite o nome do arquivo de imagem:\n");
                    scanf("%s", filename);
                    if (png2bin(filename) == 0) {
                        //ponteiro da string que será mandada para o driver
                        store_image(hps_virtual, caminho_img_bin);
                        exibir_imagem(hps_virtual, caminho_img_bin);
                        printf("Nova imagem foi reenviada\n");
                    } else {
                        printf("Erro ao reenviar imagem\n Confira o nome do arquivo digitado.");
                        break;
                    }

                    int esperado;
                    printf("\nResultado esperado:\n");
                    scanf("%d", &esperado);
                    
                    iniciar(hps_virtual);

                    int result = get_resultado(hps_virtual);

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
                reset(hps_virtual);

                printf("Abrindo painel de desenho no VGA...\n");
                executar_painel_desenho_vga(hps_virtual);

                store_image(hps_virtual, caminho_desenho);
                exibir_imagem(hps_virtual, caminho_desenho);

                iniciar(hps_virtual);

                int result = get_resultado(hps_virtual);

                printf("\nResultado da inferência:%d\n",result);
                break;
            }
 
            // Benchmark

 
            // Outras opções
            case 4:
            {
                int loop1 = 1;
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
                            reset(hps_virtual);
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
                            int custom_inst = ler_binario_para_decimal("Digite a instrução personalizada (em binario)", 32);

                            instruction(hps_virtual, custom_inst);
                            enable(hps_virtual);

                            printf("\nInstrução personalizada enviada\n");
                            break;
                        }
 
                        // Enviar pixel (antigo case 9)
                        case 5:
                        {
                            int endereco  = ler_binario_para_decimal("Digite o endereço para o pixel (em binario)", 10);
                            int pixel = ler_binario_para_decimal("Digite o valor do pixel (em decimal): (em binario)", 8);

                            str_img(hps_virtual, endereco, pixel);
                            enable(hps_virtual);

                            printf("\nPixel enviado\n");
                            break;
                        }
 
                        // Enviar peso (antigo case 8)
                        case 6:
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
 
                        // Enviar bias (antigo case 10)
                        case 7:
                        {
                            int endereco = ler_binario_para_decimal("Digite o endereço para o bias (em binario)", 7);
                            int bias = ler_binario_para_decimal("Digite o valor do bias (em binario)", 16);

                            str_bias(hps_virtual, endereco, bias);
                            enable(hps_virtual);

                            printf("\nBias enviado\n");
                            break;
                        }
 
                        // Enviar beta (antigo case 11)
                        case 8:
                        {
                            int endereco = ler_binario_para_decimal("Digite o endereço para o beta (em binario)", 11);
                            int beta = ler_binario_para_decimal("Digite o valor do beta (em binario)", 16);

                            str_beta(hps_virtual, endereco, beta);
                            enable(hps_virtual);

                            printf("\nBeta enviado\n");
                            break;
                        }
 
                        // Confirmar operação enviada (antigo case 12)
                        case 9:
                        {
                            long int ope = confirmar(hps_virtual);
                            printf("\nOperação enviada: \n");
                            imprimir_decimal_para_binario(ope);
                            break;
                        }
 
                        // Reenviar imagem prédefinida (antigo case 13)
                        case 10:
                        {
                            store_image(hps_virtual, "predef_6.bin");
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


