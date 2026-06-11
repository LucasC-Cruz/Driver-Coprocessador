#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <termios.h>
#include <stdbool.h>


#include "mouse.h"
#include "vga.h" 

#define PIO_VGA_OFFSET      0x0090 
#define PIO_VGA_DONE_OFFSET 0x00A0 

// Configurações lógicas da imagem e renderização
#define IMG_SIZE 28
#define SCALE_FACTOR 8  
#define OFFSET_X ((320 - (IMG_SIZE * SCALE_FACTOR)) / 2) 
#define OFFSET_Y ((240 - (IMG_SIZE * SCALE_FACTOR)) / 2)

// ponteiros para o enderecamento do pio
static volatile uint32_t *vga_pio_ptr = NULL;
static volatile uint32_t *vga_done_ptr = NULL;

static void enviar_pixel(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b) {
    uint32_t pixel_cmd = x | (y << 9) | (r << 17) | (g << 20) | (b << 23);
    *vga_pio_ptr = pixel_cmd;
    *vga_pio_ptr = pixel_cmd | (1 << 26); // Enable = 1
    while ((*vga_done_ptr & 0x1) == 0);
}



// Renderiza o macro-pixel (bloco de 8x8) na VGA
static void atualizar_bloco_vga_cor(uint8_t tela_desenho[IMG_SIZE][IMG_SIZE], int img_x, int img_y, bool apagar) {
    uint32_t vga_x, vga_y, novo_pixel_tela;
    
    int maxi, meio, mini;
    if (apagar) {
        maxi = -7;
        meio = -3;
        mini = -1;
    } else {
        maxi = 7;
        meio = 3;
        mini = 1;
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
static void selecao_cursor(int img_x, int img_y, uint32_t r, uint32_t g, uint32_t b) {
    uint32_t vga_x, vga_y;

        for (int e_y = 0; e_y < SCALE_FACTOR; e_y++) {
            for (int e_x = 0; e_x < SCALE_FACTOR; e_x++) {
                if (e_x == 0 || e_x == (SCALE_FACTOR-1) || e_y == 0 || e_y == (SCALE_FACTOR-1)){
                    vga_x = OFFSET_X + (img_x * SCALE_FACTOR) + e_x;
                    vga_y = OFFSET_Y + (img_y * SCALE_FACTOR) + e_y;
                    enviar_pixel(vga_x, vga_y, r, g, b);
            }
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

// Inicializa o plano de fundo da tela
static void limpar_tela_completa() {
    for(uint32_t y = 0; y < 240; y++) {
        for(uint32_t x = 0; x < 320; x++) {
            enviar_pixel(x, y, 0, 0, 0);
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
    uint8_t tela_desenho[IMG_SIZE][IMG_SIZE] = {0};

    int mouse_fd;

    mouse_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    if (mouse_fd == -1) {
        perror("Erro ao abrir o dispositivo do mouse (/dev/input/mice)");
        return 1;
    }


    vga_pio_ptr  = (volatile uint32_t *)(hps_virtual + (PIO_VGA_OFFSET));
    vga_done_ptr = (volatile uint32_t *)(hps_virtual + (PIO_VGA_DONE_OFFSET));

    printf("Inicializando o painel de desenho...\n");
    limpar_tela_completa();

    printf("\n==================================================\n");
    printf(" - Desenhe livremente no monitor VGA\n");
    printf(" - Click Scroll do mouse: Salva a imagem desenhada\n");
    printf("==================================================\n");

    int cursor_x = IMG_SIZE / 2;
    int cursor_y = IMG_SIZE / 2;
    int prev_x = cursor_x;
    int prev_y = cursor_y;

    selecao_cursor(cursor_x, cursor_y, 0, 7, 0); // Cursor verde

    uint8_t pacote_mouse[3];
    
        while (1) {

            //lê 3 bytes do file descriptor do mouse para pacote_mouse
            int bytes_lidos = read(mouse_fd, pacote_mouse, sizeof(pacote_mouse));
            if (bytes_lidos == 3) {
                //o primeiro byte são os botoes
                uint8_t botoes = pacote_mouse[0];
                //o 2 e 3 byte são os deslocamentos x e y
                int8_t delta_x = (int8_t)pacote_mouse[1];
                int8_t delta_y = (int8_t)pacote_mouse[2];

                //salvando anterior, necessario para restaurar pixel sobrescrito pelo cursor
                prev_x = cursor_x;
                prev_y = cursor_y;

                //incrementando a posição do cursor com base no deslocamento 
                if (delta_x > 1)  cursor_x++;
                if (delta_x < -1) cursor_x--;
                if (delta_y > 1)  cursor_y--; 
                if (delta_y < -1) cursor_y++;

                //definindo os limtes de onde o cursor pode ir
                if (cursor_x < 0) cursor_x = 0;
                if (cursor_x >= IMG_SIZE) cursor_x = IMG_SIZE - 1;
                if (cursor_y < 0) cursor_y = 0;
                if (cursor_y >= IMG_SIZE) cursor_y = IMG_SIZE - 1;

                if (cursor_x != prev_x || cursor_y != prev_y) {
                    restaurar_bloco_original( tela_desenho, prev_x, prev_y);
                    selecao_cursor(cursor_x, cursor_y, 7, 0, 7);
                }

                int clique_esquerdo = botoes & 0x01;
                int clique_direito  = botoes & 0x02;
                int clique_scroll  = botoes & 0x04;

                if (clique_esquerdo) {
                    atualizar_bloco_vga_cor(tela_desenho, cursor_x, cursor_y, 0);
                    selecao_cursor(cursor_x, cursor_y, 0, 7, 0);
                    
                } 
                else if (clique_direito) {

                    atualizar_bloco_vga_cor(tela_desenho, cursor_x, cursor_y, 1);
                    selecao_cursor(cursor_x, cursor_y, 7, 0, 0);
                } else if (clique_scroll) {
                    salvar_imagem_bin(tela_desenho);
                    close(mouse_fd);
                    return 0;
                }
            }
        }

}

