#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

#include "vga.h"

#define PIO_VGA_OFFSET      0x0090 
#define PIO_VGA_DONE_OFFSET 0x00A0 

// Definições do formato da imagem original
#define IMG_SIZE 28
#define TOTAL_BYTES 784 // 28 * 28

// Configuração de dimensionamento para visualização na tela de 320x240
#define SCALE_FACTOR 8  // Cada pixel vira um bloco de 8x8 (28 * 8 = 224 pixels na tela)
#define OFFSET_X ((320 - (IMG_SIZE * SCALE_FACTOR)) /2) // Centraliza horizontalmente (48)
#define OFFSET_Y ((240 - (IMG_SIZE * SCALE_FACTOR)) /2) // Centraliza verticalmente (8)

// Função que envia o comando de pixel e aguarda de forma síncrona o sinal de "done" da FPGA
void enviar_pixel(volatile uint32_t *vga_ptr, uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b, volatile uint32_t *vga_done_ptr) {
    // Monta o pacote de dados com o bit de enable em 0
    uint32_t pixel_cmd = x | (y << 9) | (r << 17) | (g << 20) | (b << 23);
    
    // 1. Envia as coordenadas e cores para o registrador
    *vga_ptr = pixel_cmd;
    
    // 2. Levanta o sinal de enable (Bit 26) para iniciar a escrita na memória M10K
    *vga_ptr = pixel_cmd | (1 << 26);
    
    // 3. Aguarda o hardware (lsu_controller/VGA) responder com done = 1
    while ((*vga_done_ptr & 0x1) == 0) {
        // Polling ativo aguardando a FPGA terminar a operação
    }
    
    // 4. Desativa o sinal de enable voltando o Bit 26 para 0
    *vga_ptr = pixel_cmd;
    
    // 5. Aguarda a máquina de estados da FPGA limpar a flag e retornar para o estado IDLE
    while ((*vga_done_ptr & 0x1) == 1) {
        // Polling ativo aguardando o recuo do estado
    }
}

int enviar_imagem(void *virtual_base) {
    volatile uint32_t *vga_pio_ptr = NULL;
    volatile uint32_t *vga_done_ptr = NULL;
    
    uint8_t buffer_imagem[TOTAL_BYTES];

    // --- PASSO 1: LER O ARQUIVO BINÁRIO "image" DO DISCO ---
    FILE *file_ptr;
    file_ptr = fopen("image.bin", "rb");
    if (file_ptr == NULL) {
        perror("Erro ao abrir o arquivo 'image'. Certifique-se de que ele esta no mesmo diretorio.");
        return 1;
    }

    // Lê os 784 bytes do arquivo e joga no buffer
    size_t bytes_lidos = fread(buffer_imagem, sizeof(uint8_t), TOTAL_BYTES, file_ptr);
    fclose(file_ptr);

    if (bytes_lidos != TOTAL_BYTES) {
        fprintf(stderr, "Erro: O arquivo continha apenas %zu bytes, esperava-se %d bytes.\n", bytes_lidos, TOTAL_BYTES);
        return 1;
    }

    // Atribui os ponteiros mapeados para as variáveis voláteis
    vga_pio_ptr  = (volatile uint32_t *)(virtual_base + PIO_VGA_OFFSET);
    vga_done_ptr = (volatile uint32_t *)(virtual_base + PIO_VGA_DONE_OFFSET);

    //limpa tela com fundo preto
    for(uint32_t y = 0; y < 240; y++) {
        for(uint32_t x = 0; x < 320; x++) {
            enviar_pixel(vga_pio_ptr, x, y, 0, 0, 0, vga_done_ptr);
        }
    }

    // --- PASSO 4: EXIBIR A IMAGEM COM REDIMENSIONAMENTO E POLLING DE DONE ---

    for (int img_y = 0; img_y < IMG_SIZE; img_y++) {

        for (int img_x = 0; img_x < IMG_SIZE; img_x++) {
            
            // Calcula o índice linear correspondente no arquivo binário de 784 bytes
            int indice_pixel = (img_y * IMG_SIZE) + img_x;
            uint8_t pixel_original = buffer_imagem[indice_pixel];
            
            // Converte a escala de tons de cinza de 8 bits (0-255) para 3 bits (0-7)
            // Caso sua imagem já seja binária pura com valores de 0 a 7, use apenas: tom = pixel_original;
            uint32_t tom = pixel_original >> 5; 
            
            // Replicação de pixel (Upscaling por software enviando blocos de SCALE_FACTOR x SCALE_FACTOR)
            for (int e_y = 0; e_y < SCALE_FACTOR; e_y++) {
                
                for (int e_x = 0; e_x < SCALE_FACTOR; e_x++) {
                    
                    uint32_t vga_x = OFFSET_X + (img_x * SCALE_FACTOR) + e_x;
                    uint32_t vga_y = OFFSET_Y + (img_y * SCALE_FACTOR) + e_y;
                    
                    // Envia garantindo que a FPGA terminou o ciclo anterior
                    enviar_pixel(vga_pio_ptr, vga_x, vga_y, tom, tom, tom, vga_done_ptr);
                }
            }
            
        }
    }

    return 0;
}