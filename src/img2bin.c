#include <stdio.h>
#include <stdlib.h>
#include "img2bin.h"
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int png2bin(const char *input_filename) {
    int width, height, channels_in_file;
    
    unsigned char *img_data = stbi_load(input_filename, &width, &height, &channels_in_file, 1);

    if (img_data == NULL) {
        return 1;}
        
    if (width != 28 || height != 28) {
        stbi_image_free(img_data);

        return 2;
    }

    FILE *out_file = fopen("assets/image.bin", "wb");
    if (out_file == NULL) {
        stbi_image_free(img_data);
        
        return 3;
    }

    size_t bytes_to_write = width * height;
    size_t written = fwrite(img_data, 1, bytes_to_write, out_file);

    fclose(out_file);
    stbi_image_free(img_data);

    if (written != bytes_to_write) return 4;

    return 0;
}

