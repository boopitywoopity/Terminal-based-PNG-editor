#include <stdlib.h>
#include "globals.h"

WINDOW *win = NULL;
int x_buffer = 0;
int y_buffer = 0;
int loop_count = 1;
int loop_input_assigned = 0;

RGB_spectrum *colors = NULL;
unsigned int current_colors = 0;
unsigned int current_max_colors = 1000;
unsigned int color_index = START_COLOR_INDEX;
int pixel_ratio = 1;
int current_color = -1;

point_w_color **total_image = NULL;
point_w_color *user_colors = NULL;


void free_globals(int height) {
    if (total_image) {
        for (int y = 0; y < height; y++) {
            free(total_image[y]);
        }
        free(total_image);
        total_image = NULL;
    }

    if (colors) {
        for (unsigned int i = 0; i < current_colors; i++) {
            free(colors[i].point);
        }
        free(colors);
        colors = NULL;
        current_colors = 0;
    }

    if (user_colors) {
        free(user_colors);
        user_colors = NULL;
    }
}

