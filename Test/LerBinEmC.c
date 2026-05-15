#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

// mapeamento
extern void* mapear();
extern void fechar(void* hps_virtual);

extern void str_img(void* hps_virtual, volatile int endereco, int dado);

extern void str_bias(void* hps_virtual, volatile int endereco, int dado);
extern void str_beta(void* hps_virtual, volatile int endereco, int dado);

extern void str_wadress(void* hps_virtual, volatile int endereco);
extern void str_weight(void* hps_virtual, int dado);

extern void iniciar(void* hps_virtual);
extern int get_resultado(void* hps_virtual);

extern bool get_flag_busy(void* hps_virtual);
extern bool get_flag_done(void* hps_virtual);
extern bool get_flag_error(void* hps_virtual);

extern void clear_operation(void* hps_virtual);
extern void reset(void* hps_virtual);

extern void enable(void* hps_virtual);

extern int confirmar(void* hps_virtual);

void printBin(char* val, int cont, int n){
    unsigned int m = (unsigned int)n;

    printf("%s", val);
    int i;
    for (i = 31; i>= 0; i--){
        printf("%d", (m>>i) & 1);
    }
    printf("    %d", cont);
    printf("\n");
}

int main(){
    void* hps_virtual = mapear();
    printf("\n\nhps: %p \n", hps_virtual);
    
    reset(hps_virtual);
    clear_operation(hps_virtual);
    printf("Resetando coprocessador...\n");

    FILE *arquivo;
    int16_t numero; 

    volatile int predicao = get_resultado(hps_virtual);
    printf("\nResultado da inferencia:%d \n", predicao);

    //+++++++++++++++++++++++BIAS+++++++++++++++++++++
    arquivo = fopen("b_q.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }

    volatile int endereco = 0;
    while (fread(&numero, sizeof(int16_t), 1, arquivo) == 1) {
        clear_operation(hps_virtual);
        
        str_bias(hps_virtual, endereco, numero);
        
        //  printf("dado: %d\n", numero);
        int c = confirmar(hps_virtual);
        printBin("Bias: ",c, endereco);
        //printf("Instrucao de bias enviada: %d\n", c);
        enable(hps_virtual);
        volatile bool done = get_flag_done(hps_virtual); 
        while (done==0){
            done = get_flag_done(hps_virtual); 
        }

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
        clear_operation(hps_virtual);

        str_beta(hps_virtual, endereco, (volatile)numero);
        // printf("dado: %d\n", numero);
        // int c = confirmar(hps_virtual);
        // printf("Instrucao de BETA enviada: %d\n", c);
        enable(hps_virtual);
        volatile bool done = get_flag_done(hps_virtual); 
        while (done==0){
            done = get_flag_done(hps_virtual); 
        }
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de beta alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura de beta.\n");
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
        clear_operation(hps_virtual);

        str_wadress(hps_virtual, endereco);
        enable(hps_virtual);

        str_weight(hps_virtual, (volatile)numero);
        enable(hps_virtual);
        volatile bool done = get_flag_done(hps_virtual); 
        while (done==0){
            done = get_flag_done(hps_virtual); 
        }
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de pesos alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura dos pesos.\n");
    }
    fclose(arquivo);
    //+++++++++++++++++++++++IMAGEM+++++++++++++++++++++
    arquivo = fopen("imagem_9.bin", "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }
    uint8_t pixel;
    endereco = 0;
    while (fread(&pixel, sizeof(uint8_t), 1, arquivo) == 1) {
        clear_operation(hps_virtual);

        str_img(hps_virtual, endereco, (volatile)pixel);
        // printf("dado: %d\n", pixel);
        // int c = confirmar(hps_virtual);
        // printf("Instrucao de IMAGEM enviada: %d\n", c);
        enable(hps_virtual);
        volatile bool done = get_flag_done(hps_virtual); 
        while (done==0){
            done = get_flag_done(hps_virtual); 
        }
        // volatile bool done = get_flag_done(hps_virtual); 
        // printf("\nFlag de done: %d \n", done);
        endereco++;
    }

    if (feof(arquivo)) {
        printf("\nFim do arquivo de imagem alcançado com sucesso.\n");
    } else if (ferror(arquivo)) {
        printf("\nOcorreu um erro durante a leitura da imagem.\n");
    }
    fclose(arquivo);
    //++++++++++++++++++++++++INFERENCIA++++++++++++++++++
    clear_operation(hps_virtual);
    iniciar(hps_virtual);
    int c = confirmar(hps_virtual);
    printf("Instrucao de INICIAR enviada: %d\n", c);
    enable(hps_virtual);
    volatile bool done, busy, error;
    
    done = get_flag_done(hps_virtual); 
    printf("\nFlag de done: %d \n", done);
    while (done==0){
        done = get_flag_done(hps_virtual); 
        printf("Flag de done INFERENCIA: %d \n", done);

    }

    busy = get_flag_busy(hps_virtual); 
    printf("Flag de busy: %d \n", busy);
    
    error = get_flag_error(hps_virtual);    
    printf("Flag de erro: %d \n", error);

    predicao = get_resultado(hps_virtual);
    printf("\nResultado da inferencia:%d \n", predicao);

    fechar(hps_virtual);
    printf("\nMapeamento encerrado.\n");

    return 0;
}
