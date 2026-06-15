#include <stdlib.h>
#include <stdint.h>

#ifndef PIO_H
#define PIO_H

#define PIO_VGA_OFFSET      0x0090 
#define PIO_VGA_DONE_OFFSET 0x00A0 

// ponteiros para o enderecamento do pio
extern volatile uint32_t *vga_done_ptr;
extern volatile uint32_t *vga_pio_ptr;

#endif 