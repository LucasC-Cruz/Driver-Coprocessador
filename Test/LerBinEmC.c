#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// mapeamento
extern void* mapear();
extern void fechar(void* hps_virtual);

extern void str_img(void* hps_virtual, int endereco, int dado);

extern void str_bias(void* hps_virtual, int endereco, int dado);
extern void str_beta(void* hps_virtual, int endereco, int dado);

extern void str_wadress(void* hps_virtual, int endereco);
extern void str_weight(void* hps_virtual, int dado);

extern void iniciar(void* hps_virtual);
extern int get_resultado(void* hps_virtual);

extern bool get_flag_busy(void* hps_virtual);
extern bool get_flag_done(void* hps_virtual);
extern bool get_flag_error(void* hps_virtual);

extern void clear_operation(void* hps_virtual);
extern void reset(void* hps_virtual);

int main(){
    void* hps_virtual = mapear();
    printf(" %p ", hps_virtual);
    
    reset(hps_virtual);
    clear_operation(hps_virtual);
    printf("Resetando coprocessador...\n");

    FILE *arquivo;
    int16_t numero; 


    //+++++++++++++++++++++++BIAS+++++++++++++++++++++
    arquivo = fopen("b_q.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }

    int endereco = 0;
    while (fread(&numero, sizeof(int16_t), 1, arquivo) == 1) {
        str_bias(hps_virtual, endereco, numero);
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de bias alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura de bias.\n");
    }
    fclose(arquivo);
    //+++++++++++++++++++++BETA++++++++++++++++++++++
    arquivo = fopen("beta_q.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }

    endereco = 0;
    while (fread(&numero, sizeof(int16_t), 1, arquivo) == 1) {
        str_beta(hps_virtual, endereco, numero);
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de betinhas alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura de betinhas (mogados).\n");
    }
    fclose(arquivo);
    //+++++++++++++++++++++++PESOS+++++++++++++++++++++
    arquivo = fopen("W_in_q.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }

    endereco = 0;
    while (fread(&numero, sizeof(int16_t), 1, arquivo) == 1) {
        str_wadress(hps_virtual, endereco);
        str_weight(hps_virtual, numero);
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de pesos alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura dos pesos.\n");
    }
    fclose(arquivo);
    //+++++++++++++++++++++++IMAGEM+++++++++++++++++++++
    arquivo = fopen("imagem_4.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }
    uint8_t pixel;
    endereco = 0;
    while (fread(&pixel, sizeof(uint8_t), 1, arquivo) == 1) {
        str_img(hps_virtual, endereco, pixel);
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de imagem alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura da imagem.\n");
    }
    fclose(arquivo);
    //++++++++++++++++++++++++INFERENCIA++++++++++++++++++
    iniciar(hps_virtual);
    bool done, busy, error;
    done = get_flag_done(hps_virtual); 
    printf("\nFlag de done: %d \n", done);

    busy = get_flag_busy(hps_virtual); 
    printf("Flag de busy: %d \n", busy);
    
    error = get_flag_error(hps_virtual);    
    printf("Flag de erro: %d \n", error);

    int predicao = get_resultado(hps_virtual);
    printf("\nResultado da inferencia:%d \n", predicao);

    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");

    return 0;
}