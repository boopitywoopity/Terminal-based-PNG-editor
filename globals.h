#ifndef GLOBALS_H
#define GLOBALS_H

#include "util.h"

// global variable declaration goes here

extern WINDOW *win;
extern int x_buffer;
extern int y_buffer;
extern int loop_count;
extern int loop_input_assigned;

// This contains all colors in the image, each color can be bound to multiple points
extern RGB_spectrum *colors;
extern unsigned int current_colors;

extern unsigned int current_max_colors;
extern unsigned int color_index;

extern int pixel_ratio;
extern point_w_color **total_image;

extern int current_color;
extern point_w_color *user_colors;

void free_globals(int height);

#endif // GLOBALS_H
