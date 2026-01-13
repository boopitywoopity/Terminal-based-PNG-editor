#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "lib/stb_image.h"
#include "util.h"

static image_container generate_default(const int width, const int height){
    image_container img = {
        .x_buffer = 1,
        .y_buffer = 1,
        .current_max_colours = 1000,
        .width = width,
        .height = height,
        .pixel_ratio = 1,
        .colour_index = 0,
        .current_colours = 0,
    };

    img.root = NULL;

    img.total_image = malloc(sizeof(uint32_t) * (height+1));

    if (img.total_image == NULL) { // there was a failure to allocate enough memory for the total image
        endwin();
        printf("There was an issue allocating memory for the total image\n");
        exit(EXIT_FAILURE);
    }

    return img;
}

image_container load_image(unsigned char *image, unsigned int width, unsigned int height){
    image_container img = generate_default(width, height);

    for (int y = 0; y < height; y++) {
        img.total_image[y] = malloc(sizeof(uint32_t) * (width+1)); // alocates the memory per line

        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            uint32_t value =
                ((uint32_t)image[index + 0] << 24) | // R
                ((uint32_t)image[index + 1] << 16) | // G
                ((uint32_t)image[index + 2] << 8)  | // B
                ((uint32_t)image[index + 3]);        // A
            img.total_image[y][x] = value;
            if (!contains(&img.root, value) && image[index+3] != 0){
                insert(&img.root, value);
                img.current_colours += 1;
            }
        }
    }
    return img;
}
