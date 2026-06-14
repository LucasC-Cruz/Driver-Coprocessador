#ifndef VGA_H
#define VGA_H

int exibir_imagem(void *virtual_base, const char *imagem);
void enviar_pixel(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b);
void limpar_tela();
 
#endif 