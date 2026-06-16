#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#include <math.h>


#include "colors.h"
#include "api.h"
#include "img2bin.h"
#include "vga.h"
#include "mouse.h"
#include "pio.h"


#include <time.h>

volatile uint32_t *vga_pio_ptr; 
volatile uint32_t  *vga_done_ptr;

#define LIST_FILE    "src/arquivos_1_bin.txt"
//definindo range máximo para sorteio
#define RAND_MAX_IDX 10000
#define OUTPUT_FILE  "src/saida.txt"
#define OUTPUT_FILE2  "src/saida2.txt"
#define N 10

void salvar_matriz_csv(int matriz[N][N], const char *caminho) {
    FILE *out = fopen(caminho, "w");
    if (!out) { perror("Erro ao criar CSV"); return; }
 
    fprintf(out, "Real\\Predito");
    for (int j = 0; j < N; j++) fprintf(out, ";Pred_%d", j);
    fprintf(out, "\n");
 
    for (int i = 0; i < N; i++) {
        fprintf(out, "Real_%d", i);
        for (int j = 0; j < N; j++) fprintf(out, ";%d", matriz[i][j]);
        fprintf(out, "\n");
    }
    fclose(out);
}

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

void salvar_relatorio_acertos(int matriz[N][N], const char *caminho) {
    FILE *out = fopen(caminho, "w");
    if (!out) { perror("Erro ao criar arquivo"); return; }
 
    fprintf(out, "Valor Esperado;Acertos;Porcentagem de Acertos\n");
 
    for (int i = 0; i < N; i++) {
        int total = 0;
        for (int j = 0; j < N; j++) total += matriz[i][j];
        int acertos = matriz[i][i];
        float pct = (total > 0) ? (100.0f * acertos / total) : 0.0f;
        fprintf(out, "%d;%d;%.2f%%\n", i, acertos, pct);
    }
    fclose(out);
}

void salvar_metricas(double acuracia, double latencia,
                     double latenciaRealMedia, double vazao, double desvioLatencia, const char *caminho,) {
    FILE *out = fopen(caminho, "w");
    if (!out) { perror("Erro ao criar arquivo"); return; }
 
    fprintf(out, "Acurácia;Latência Teórica;Latência Média;Vazão;Desvio Latência\n");
    fprintf(out, "%.4f;%.4f;%.4f;%.4f;%.4f\n",
            acuracia, latencia, latenciaRealMedia, vazao, desvioLatencia);
 
    fclose(out);
}



int main() 
 
   int matriz[N][N];
   memset(matriz, 0, sizeof(matriz))
   char* caminho_img_bin = "assets/image.bin";
   char* caminho_desenho = "assets/desenho.bin";
   bool done, busy, error;
   int inst;
   int a; //usado para selecionar opção no segundo menu
   int loop=1;
   struct timespec inicio2, fim2;


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
   int totalClocks = (num_inst *5) + (32*18844) + (2*18844) + 10 + 2;
   //possivelmente teremos que resetar ao longo dos fluxos de inferência
   
   do
   
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


                   //número de instruções de memória * 5 +  clocks primeira camada (32 * 18844) + clocks segunda camada 2*(18844) +
                   // clocks argmax 10 + clock de controle 2

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
           case 3:
            char linha[256];
            char caminho[200];
            int val_esperado;
            int result, erro=0;
            double acuracia, latencia, desvioLatencia, vazao, tempo;
            int tempoPorInferencia[10000];
            double tempoTotal;
            float tempoClock = 0.000000002; //considerando 50 Mhz
            float latenciaRealMedia;
            int totalInferencias=0;
            int opcao = -1;

            while (opcao != 0) 
                printf("\n--- Menu de Benchmark ---\n");
                printf("1 - Modo benchmark total\n");
                printf("2 - Benchmark sorteado\n");
                printf("3 - Benchmark por pasta\n");
                printf("0 - Sair\n");
                printf("Escolha uma opcao: ");
                scanf("%d", &opcao);
                switch (opcao) 
                    case 1:
                        printf("Voce escolheu: Modo benchmark total.\n");
                        FILE* arquivo = fopen(LIST_FILE, "r");
                        FILE* saida   = fopen(OUTPUT_FILE, "r+");
                        if (arquivo == NULL || saida == NULL){
                            printf("Erro ao abrir o arquivo!");
                            return 1;
                        }

                        fprintf(saida, "Valor Esperado;Resultado;Latência;Ciclos;\n");
                    
                        while(fgets(linha, sizeof(linha), arquivo) != NULL){
                            sscanf(linha, "%[^;];%d;", caminho, &val_esperado);
                            reset(hps_virtual);
                            //manda a imagem para o driver abrir e colocar no buffer para ser enviado ao coprocessador
                            store_image(hps_virtual, caminho);
                         
                            clock_gettime(CLOCK_MONOTONIC, &inicio2);
                            //fazer inferencia
                            iniciar(hps_virtual);
                            clock_gettime(CLOCK_MONOTONIC, &fim2);
                            tempo = (fim2.tv_sec - inicio2.tv_sec) + (fim2.tv_nsec - inicio2.tv_nsec) / 1e9;
                            tempoPorInferencia[totalInferencias] = tempo;
                         
                         
                            //pega resultado
                            int result = get_resultado(hps_virtual);
                            totalInferencias+=1;
                            trim(linha);// tirar o \n que tá no arquivo


                            //coloca o resultado no arquivo de saida
                            fprintf(saida, "%d;%d;%4f;%d\n", val_esperado, result, tempo, totalClocks);
                            //colocar só o valor esperado o resultado, o tempo que demorou e o número de ciclos   
                        }

                        fclose(arquivo);

                        // ------------- PARTE DE ELABORAÇÃO DO LOG DE RESULTADO DA INFERÊNCIA ---------- // 
                        // volta para o início sem fechar
                        fseek(saida, 0, SEEK_SET);
                        //descartar a primeira linha
                        fgets(linha, sizeof(linha), saida);
                        while(fgets(linha, sizeof(linha), saida) != NULL){
                            sscanf(linha, "%*[^;];%d;%d;%f;%d", &val_esperado, &result, &tempo, &totalClocks);
                            if(val_esperado != result)
                                erro += 1;
                            //tem como extrair total inferido de cada número e quantos acertos
                            matriz[esperado][predito]++;
                            tempoTotal += tempo;
                        }
                        fclose(saida);
                       
                        salvar_matriz_csv(matriz, "matrizConfusao.txt");
                        salvar_relatorio_acertos(matriz, "acertos.txt");
                       
                        acuracia = (double)(totalInferencias-erro)/totalInferencias;
                        latencia = totalClocks * tempoClock;
                        latenciaRealMedia = tempoTotal/totalInferencias;
                        vazao = 1/(tempoTotal/totalInferencias);
                     
                        double a;
                        for (int i=0; i<totalInferencias; i++){
                            a += pow((tempoPorInferencia[i] - latenciaReal), 2);
                        }
                        desvioLatencia= sqrt(a/totalInferencias);
                        salvar_metricas(acuracia, latencia, latenciaRealMedia, vazao, desvioLatencia, "metricas.txt")
                        
                        break;
                       


                     case 2:
                        printf("Voce escolheu: Benchmark sorteado.\n");
                        int tempoPorInferencia[10000];
                        /* ── 1. Abre o .txt e lê tudo num buffer ── */
                        FILE* arquivo = fopen(LIST_FILE, "r");
                        //talvez dê um problema se o arquivo não existir
                        FILE* saida   = fopen(OUTPUT_FILE2, "r+");
                        if (arquivo == NULL || saida == NULL){
                            printf("Erro ao abrir o arquivo!");
                            return 1;
                        }

                        //truque para descobrir o tamanho do arquivo
                        fseek(list_file, 0, SEEK_END);
                        long size = ftell(list_file);
                        rewind(list_file); //voltando ao começo do arquivo

                        char *buffer = malloc(size + 1);
                        if (!buffer) { perror("malloc"); fclose(list_file); return EXIT_FAILURE; }

                        //coloca o arquivo no buffer
                        fread(buffer, 1, size, list_file);
                        buffer[size] = '\0';
                        fclose(list_file);

                        /* ── 2. Conta linhas e monta o array de ponteiros ── */
                        int n_paths = 0;

                        //lendo o buffer para descobrir o número de caminhos presentes no arquivo
                        for (long i = 0; i < size; i++)
                            if (buffer[i] == '\n') n_paths++;
                        if (size > 0 && buffer[size - 1] != '\n') n_paths++;
                        //cria array de ponteiros contendo os caminhos 
                        char **paths = malloc(n_paths * sizeof(char *));
                        if (!paths) { perror("malloc"); free(buffer); return EXIT_FAILURE; }
                        
                        //carregando as linhas na estrutura de dado (array safadinho)
                        int count = 0;
                        char *line = strtok(buffer, "\r\n");
                        while (line) {
                            if (line[0] != '\0') {
                                /* corta tudo a partir do primeiro ';' */
                                char *semi = strchr(line, ';');
                                if (semi) *semi = '\0';
                                paths[count++] = line;
                            }
                            line = strtok(NULL, "\r\n");
                        }
                        printf("Caminhos carregados: %d\n", count);

                        /* ── 3. Lê o número de sorteios ── */
                        int n_draws;
                        printf("Numero de sorteios: ");
                        if (scanf("%d", &n_draws) != 1 || n_draws <= 0) {
                            fprintf(stderr, "Entrada invalida.\n");
                            free(paths); free(buffer);
                            return EXIT_FAILURE;
                        }
                        /* ── 4. Sorteia e abre as imagens ── */
                        //esse negócio aqui é bem interessante
                        //srand é a seed rand é basicamente a partir dela que geramos uma sequência de números aleatórios
                        // time(null) ppega a hora atual do sistema em segundos, oq gera novas sementes a cada segundo
                        //:)
                        srand((unsigned)time(NULL));

                        printf("\nIniciando benchmark (%d sorteios)...\n\n", n_draws);
                        for (int i = 0; i < n_draws; i++){
                            reset(hps_virtual); 
                            int idx      = rand() % (RAND_MAX_IDX + 1);
                            int real_idx = idx % count;
                            printf("[%4d] idx=%5d -> %s\n", i + 1, idx, paths[real_idx]);
                            //aqui daríamos open, mas o que devemos fazer é passar o caminho para o driver
                            store_image(hps_virtual, paths[real_idx]);

                            clock_gettime(CLOCK_MONOTONIC, &inicio2);
                            iniciar(hps_virtual);
                            clock_gettime(CLOCK_MONOTONIC, &fim2);
                            tempo = (fim2.tv_sec - inicio2.tv_sec) + (fim2.tv_nsec - inicio2.tv_nsec) / 1e9;
                            tempoPorInferencia[totalInferencias] = tempo;
                            int result = get_resultado(hps_virtual);
                            printf("%d", result);


                        }
                        printf("\nBenchmark concluido.\n");
                        /* ── 5. Libera memória ── */
                        free(paths);
                        free(buffer);
                        break;

                    case 3:
                        printf("Voce escolheu: Benchmark por pasta.\n");

                        /* ══════════════════════════════════════════
                           1. Menu — dígito (0-9) e iterações (100-300)
                           ══════════════════════════════════════════
                        */
                        int digit, iterations;

                        printf("===== Benchmark de Acesso a Imagens =====\n\n");

                        do {
                            printf("Escolha um digito [0-9]: ");
                            scanf("%d", &digit);
                        } while (digit < 0 || digit > 9);
                    
                        do {
                            printf("Numero de iteracoes [100-300]: ");
                            scanf("%d", &iterations);
                        } while (iterations < 100 || iterations > 300);
                    
                        printf("\nDigito escolhido : %d\n", digit);
                        printf("Iteracoes        : %d\n\n", iterations);
                    
                        /* ══════════════════════════════════════════
                           2. Carrega o .txt inteiro num buffer
                           ══════════════════════════════════════════ */
                        FILE *list_file = fopen(LIST_FILE, "r");
                        if (!list_file) { perror("fopen"); return EXIT_FAILURE; }
                    
                        fseek(list_file, 0, SEEK_END);
                        long size = ftell(list_file);
                        rewind(list_file);
                    
                        char *buffer = malloc(size + 1);
                        if (!buffer) { perror("malloc"); fclose(list_file); return EXIT_FAILURE; }
                    
                        fread(buffer, 1, size, list_file);
                        buffer[size] = '\0';
                        fclose(list_file);
                    
                        /* ══════════════════════════════════════════
                           3. Filtra as linhas pelo dígito escolhido
                              Formato: /home/.../testes/<d>/<img>.png;<d>;
                              O filtro é o label após o último ';', ex: ";9;"
                           ══════════════════════════════════════════ */
                    
                        /* monta o sufixo de filtro, ex: ";9;" */
                        char filter[8];
                        snprintf(filter, sizeof(filter), ";%d;", digit);
                    
                        /* primeiro passo: conta quantas linhas batem com o filtro */
                        int n_paths = 0;
                        char *p = buffer;
                        while (*p) {
                            char *nl = strchr(p, '\n');
                            char *end = nl ? nl : p + strlen(p);
                            /* procura o filtro dentro desta linha */
                            if (end > p) {
                                char saved = *end; *end = '\0';
                                if (strstr(p, filter)) n_paths++;
                                *end = saved;
                            }
                            p = nl ? nl + 1 : end;
                        }
                    
                        if (n_paths == 0) {
                            fprintf(stderr, "Nenhum caminho encontrado para o digito %d.\n", digit);
                            free(buffer);
                            return EXIT_FAILURE;
                        }
                    
                        char **paths = malloc(n_paths * sizeof(char *));
                        if (!paths) { perror("malloc"); free(buffer); return EXIT_FAILURE; }
                    
                        /* segundo passo: preenche o array apontando para cada linha filtrada
                           e corta a string no primeiro ';' para ficar só o caminho limpo     */
                        int count = 0;
                        p = buffer;
                        while (*p) {
                            char *nl  = strchr(p, '\n');
                            char *end = nl ? nl : p + strlen(p);
                        
                            if (end > p) {
                                char saved = *end; *end = '\0';
                                if (strstr(p, filter)) {
                                    /* termina a string no ';' para remover ";d;" do final */
                                    char *semi = strchr(p, ';');
                                    if (semi) *semi = '\0';
                                    paths[count++] = p;
                                }
                                /* não restaura '*end': o '\0' serve de separador permanente */
                            }
                        
                            p = nl ? nl + 1 : end;
                        }
                    
                        printf("Caminhos encontrados para digito %d: %d\n\n", digit, count);
                
                        if (count < iterations){
                            printf("Há um número maior de iterações requisitadas do que o número de iteráveis, favor diminuir ou adicionar mais imagens\n");
                            printf("Número de iterações requisitadas: %d", iterations);
                            printf("Número de iteráveis: %d", count);
                            free(paths);
                            free(buffer);
                            break;
                        }
                        /* ══════════════════════════════════════════
                           4. Sorteia e abre as imagens
                           ══════════════════════════════════════════ */
                        srand((unsigned)time(NULL));
                    
                        printf("Iniciando benchmark (%d iteracoes)...\n\n", iterations);
                    
                        for (int i = 0; i < iterations; i++) {
                            reset(hps_virtual); 
                            int idx      = rand() % (RAND_MAX_IDX + 1);   /* 0 .. 10000     */
                            int real_idx = idx % count;                    /* mapeia ao array */
                        
                            printf("[%3d] idx=%5d -> %s\n", i + 1, idx, paths[real_idx]);

                            store_image(hps_virtual, paths[real_idx]);
                            iniciar(hps_virtual);
                            int result = get_resultado(hps_virtual);
                            printf("%d", result);
                        }
                    
                        printf("\nBenchmark concluido.\n");
                    
                        /* ══════════════════════════════════════════
                           5. Libera memória
                           ══════════════════════════════════════════ */
                        free(paths);
                        free(buffer);
                        break;

                    case 0:
                        printf("Retornando..\n");
                        break;

                    default:
                        printf("Opcao invalida! Tente novamente.\n");
                 
             

          
               printf("=== Resultados ===\n");
               printf("Tempo total:          %f s\n",    tempo);
               printf("Acuracia:             %f %%\n",   acuracia * 100);
               printf("Latencia teorica:     %f s\n",    latencia);
               printf("Latencia real:        %f s\n",    latenciaReal);
               printf("Vazao:                %f img/s\n", vazao);
               printf("Desvio: %f", desvioLatencia);  
               printf("%d", totalInferencias);

               //           
               break;
           

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
       
    while (loop==1);
  
   // fechar(hps_virtual);
   printf("\nMapeamento encerrado.\n");
  
   return 0;

