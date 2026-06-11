#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <termios.h>
#include <stdbool.h>
#include <linux/input.h>

#include "mouse.h"
#include "vga.h" 
#include "pio.h" 


// Configurações lógicas da imagem e renderização
#define IMG_SIZE 28
#define SCALE_FACTOR 8  
#define OFFSET_X 48//((320 - (IMG_SIZE * SCALE_FACTOR)) / 2) 
#define OFFSET_Y 8//((240 - (IMG_SIZE * SCALE_FACTOR)) / 2)

// Renderiza o macro-pixel (bloco de 8x8) na VGA
static void pincel(uint8_t tela_desenho[IMG_SIZE][IMG_SIZE], int img_x, int img_y, bool desenhando) {
    uint32_t vga_x, vga_y, novo_pixel_tela;
    
    int maxi, meio, mini;
    if (desenhando) {
        maxi = 5;
        meio = 2;
        mini = 1;
    } else {
        maxi = -7;
        meio = -2;
        mini = -1;
    }

    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            int vizinho_x = img_x + i;
            int vizinho_y = img_y + j;
            if ((vizinho_x < 0 || vizinho_x >= IMG_SIZE || vizinho_y < 0 || vizinho_y >= IMG_SIZE)) {
                continue;
            }
            //calculo do valor dos pixeis vizinhos
            if (i == 0 && j == 0) {
                novo_pixel_tela = maxi;
            } else if ((i == 0 && (j == -1 || j == 1)) || (j == 0 && (i == -1 || i == 1))) {
                novo_pixel_tela = meio;
            } else {
                novo_pixel_tela = mini;
            }
            
            //soma com o pixel atual para saber o novo valor
            int n = (int)tela_desenho[img_y+j][img_x+i] + novo_pixel_tela;

            //pra não estourar o valor maximo e minimo do pixel
            if (n>7) {
                n = 7;
            } else if (n<0) {
                n = 0;
            }

            //pra não acessar fora da tela de desenho
            tela_desenho[img_y+j][img_x+i] = n;

            //loop pra pintar todos os pixeis do vga
            for (int e_y = 0; e_y < SCALE_FACTOR; e_y++) {
                for (int e_x = 0; e_x < SCALE_FACTOR; e_x++) {
                    vga_x = OFFSET_X + ((vizinho_x) * SCALE_FACTOR) + e_x;
                    vga_y = OFFSET_Y + ((vizinho_y) * SCALE_FACTOR) + e_y;
                    enviar_pixel(vga_x, vga_y, n, n, n);
                }
            }
        }
    }
}

//desenha só as bordas do quadrado
static void selecao_cursor(uint8_t tela_desenho[IMG_SIZE][IMG_SIZE], int img_x, int img_y, uint32_t r, uint32_t g, uint32_t b) {
    uint32_t vga_x, vga_y;
    int n = tela_desenho[img_y][img_x];

    for (int e_y = 0; e_y < SCALE_FACTOR; e_y++) {
        for (int e_x = 0; e_x < SCALE_FACTOR; e_x++) {
            vga_x = OFFSET_X + (img_x * SCALE_FACTOR) + e_x;
            vga_y = OFFSET_Y + (img_y * SCALE_FACTOR) + e_y;
            if (((e_x == 0) || (e_x == 7) || (e_y == 0) || (e_y == 7))){
                enviar_pixel(vga_x, vga_y, r, g, b);
            } else {enviar_pixel(vga_x, vga_y, n, n, n);}
            
        }
    }
}

// Restaura a cor de fundo ou tinta original da matriz
static void restaurar_bloco_original(uint8_t tela_desenho[IMG_SIZE][IMG_SIZE], int img_x, int img_y) {
    int n = tela_desenho[img_y][img_x];

    for (int e_y = 0; e_y < SCALE_FACTOR; e_y++) {
        for (int e_x = 0; e_x < SCALE_FACTOR; e_x++) {
            uint32_t vga_x = OFFSET_X + (img_x * SCALE_FACTOR) + e_x;
            uint32_t vga_y = OFFSET_Y + (img_y * SCALE_FACTOR) + e_y;
            enviar_pixel(vga_x, vga_y, n, n, n);
        }
    }
}

// Função de salvamento em arquivo binário bruto
static void salvar_imagem_bin(uint8_t tela_desenho[IMG_SIZE][IMG_SIZE]) {
    printf("\n\nSalvando imagem em 'assets/desenho.bin'...");
    FILE *file_ptr = fopen("assets/desenho.bin", "wb");
    if (file_ptr == NULL) {
        perror("\nErro ao criar o arquivo desenho.bin");
        return;
    }
    uint8_t byte_conversao;
    for (int y = 0; y < IMG_SIZE; y++) {
        for (int x = 0; x < IMG_SIZE; x++) {
            if (tela_desenho[y][x]==7) byte_conversao=255;
            else byte_conversao = tela_desenho[y][x]*36;
            fwrite(&byte_conversao, sizeof(uint8_t), 1, file_ptr);
        }
    }
    fclose(file_ptr);
}

//inicializa a tela de desenho
int executar_painel_desenho_vga(void *hps_virtual) {
    //tela para desenho
    uint8_t tela_desenho[IMG_SIZE][IMG_SIZE] = {{0}};

    //abrindo fd do mouse
    const char *dev = "/dev/input/event0"; 
    int fd;

    //struct já definida pelo linux
    ///gera input_event={time,type,code,value}, que descreve inputs do mouse
    struct input_event evento;
    ssize_t pacote;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir o dispositivo");
        return EXIT_FAILURE;
    }

    printf("Inicializando o painel de desenho...\n");
    limpar_tela();

    printf("\n==================================================\n");
    printf(" - Desenhe livremente no monitor VGA\n");
    printf(" - Click Scroll do mouse: Salva a imagem desenhada\n");
    printf("==================================================\n");

    int cursor_x = IMG_SIZE / 2;
    int cursor_y = IMG_SIZE / 2;
    int prev_x = cursor_x;
    int prev_y = cursor_y;

    selecao_cursor(tela_desenho, cursor_x, cursor_y, 0, 7, 0); // Cursor verde

    bool desenhando=0;
    bool apagando=0;

    
    while (1) { 

        //lendo o pacote 
        pacote = read(fd, &evento, sizeof(struct input_event));
    
        if (pacote < (ssize_t)sizeof(struct input_event)) {
            perror("Erro ao ler evento completo");
            close(fd);
            return EXIT_FAILURE;
        }
        
        prev_x = cursor_x;
        prev_y = cursor_y;
        //incrementando a posição do cursor com base no deslocamento 
       if (evento.type == EV_REL) {
            if (evento.code == REL_X) {
                if (evento.value >= 1) cursor_x++;

                else if (evento.value <= 1) cursor_x--;
            } else if (evento.code == REL_Y) {
                if (evento.value >= 1) cursor_y++;
                else if (evento.value <= 1) cursor_y--;
            }
        }
        
        //salvando anterior, necessario para restaurar pixel sobrescrito pelo cursor
        
        // Processa eventos de botões
        if (evento.type == EV_KEY) {
            if (evento.code == BTN_LEFT) {
                if (evento.value) {
                    desenhando = 1;
                } else desenhando = 0;

            } else if (evento.code == BTN_RIGHT) {
                if (evento.value) {
                    apagando =1;
                }else apagando =0;

            } else if (evento.code == BTN_MIDDLE) {
                if (evento.value) {
                    salvar_imagem_bin(tela_desenho);
                    close(fd);
                    return 0;
                } 
            }
        }
        
        
        //definindo os limtes de onde o cursor pode ir

        if (cursor_x < 0) cursor_x = 0;
        if (cursor_x >= IMG_SIZE) cursor_x = IMG_SIZE - 1;
        if (cursor_y < 0) cursor_y = 0;
        if (cursor_y >= IMG_SIZE) cursor_y = IMG_SIZE - 1;

        if (desenhando){
            pincel(tela_desenho, cursor_x, cursor_y, 1);
            selecao_cursor(tela_desenho, cursor_x, cursor_y, 0, 7, 0);
        } 
        else if (apagando){
            pincel(tela_desenho, cursor_x, cursor_y, 0);
            selecao_cursor(tela_desenho, cursor_x, cursor_y, 7, 0, 0);
        } 

        if (cursor_x != prev_x || cursor_y != prev_y) {
            restaurar_bloco_original( tela_desenho, prev_x, prev_y);
            selecao_cursor(tela_desenho, cursor_x, cursor_y, 7, 0, 7);
        }
        
    }
}

