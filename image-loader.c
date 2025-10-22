#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/stb_image.h"
#include "util.h"
#include "globals.h"

void load_image(unsigned char *image, int width, int height){
    colors = malloc(sizeof(RGB_spectrum) * current_max_colors);

    total_image = malloc(sizeof(point_w_color *) * (height+1));

    if (total_image == NULL) { // there was a failur to allocate enough memory for the total image
        endwin();
        printf("There was an issue allocating memory for the total image\n");
        exit(EXIT_FAILURE);
    }

    for (int y = 0; y < height; y++) {
        total_image[y] = malloc(sizeof(point_w_color) * (width+1)); // alocates the memory per line
        // total_image[y] = malloc(sizeof( point_w_color *) * 400000); // alocates the memory per line
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            // fprintf(stderr, "current index: (%d, %d), %d\n", x, y, index);
            total_image[y][x].r = image[index]; // makes the variables for the point
            total_image[y][x].g = image[index + 1];
            total_image[y][x].b = image[index + 2];
            total_image[y][x].a = image[index + 3];

            // this makes sure the alpha (transparency) is not set to 0 which
            // would make the pixel invisible
            if (image[index + 3] != 0) {
                // fprintf(stderr, "current_colors: %d\n", current_colors);

                // create a new color, if the color already exists just ignore this and use the existing one, otherwise use it
                // a better way to do this might be to hash the rgb values and then look to see if it exists
                RGB_spectrum color;
                color.r = image[index];
                color.g = image[index + 1];
                color.b = image[index + 2];
                color.length = 0;

                // fprintf(stderr, "color: (%d, %d, %d) ", color.r, color.g, color.b);

                // this color already exists, bind the point to the color
                int color_position = colors_contains(color);
                if (color_position != -1) {
                    // create a new point and add it to an existing color
                    xy_point point;
                    point.x = x;
                    point.y = y;

                    // adds the new point to the points of this color
                    if (colors[color_position].max_length == colors[color_position].length){
                        colors[color_position].max_length *= 2;
                        colors[color_position].point = realloc(colors[color_position].point, sizeof(xy_point)*colors[color_position].max_length);
                    }
                    colors[color_position].point[colors[color_position].length++] = point;
                    // fprintf(stderr, "exists at index: %d\n", color_position);
                }
                // this color does not exist bind the new color to the colors and then bind the current location to the color
                else {
                    // fprintf(stderr, "does not exist\n");
                    if (current_colors+1 == INT_MAX){ // you have too many colors, kill the program
                        endwin();
                        stbi_image_free(image);
                        // Free all the colors
                        for (int i = 0; i < current_colors; i++) {
                            free(colors[i].point);
                        }

                        free(colors);
                        free(total_image);
                        printf("The image you are attempting to load has esceeded the maximum ammount of colors available\n");

                        exit(EXIT_FAILURE);
                    }

                    if (current_colors == current_max_colors) { // you have exceeded the maximum ammount of colors that is currently allocated, resize the array
                        // fprintf(stderr, "resize needed, current size: %d, new current size: %d\n", current_max_colors, current_max_colors*2);
                        current_max_colors *= 2;
                        colors = realloc(colors, sizeof(RGB_spectrum)*current_max_colors);
                    }

                    // create another point
                    xy_point point;
                    point.x = x;
                    point.y = y;

                    color.point = malloc(sizeof(xy_point) * color.max_length);
                    color.point[color.length++] = point;
                    color.max_length = MAX_STARTING_POINT_COUNT;
                    colors[current_colors++] = color;
                }
            }
        }
    }

}
