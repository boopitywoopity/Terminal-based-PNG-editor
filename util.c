#include <stdlib.h>
#include <ncurses.h>
#define START_COLOR_INDEX 27

// define a maximum possible point count for initial point creation
#define MAX_STARTING_POINT_COUNT 128

typedef struct {
        int x;
        int y;
} xy_point;

typedef struct {
        unsigned int r;
        unsigned int g;
        unsigned int b;
        xy_point *point;
        unsigned int length;
        unsigned int max_length;
        unsigned int code;
} RGB_spectrum ;

typedef struct {
        unsigned int r;
        unsigned int g;
        unsigned int b;
        unsigned int a;
} point_w_color;

// variables
WINDOW *win;
int x_buffer = 0;
int y_buffer = 0;
int loop_count = 1;
int loop_input_assigned = 0;

// This contains all colors in the image, each color can be bound to multiple points
RGB_spectrum *colors;
unsigned int current_colors = 0;
unsigned int current_max_colors = 1000;
unsigned int color_index = START_COLOR_INDEX;

int pixel_ratio = 1;
point_w_color **total_image;

int current_color = -1;
point_w_color *user_colors;

#include <limits.h>
// Define the maximum range of your long integers
#define LONG_INT_MAX 1000000

// Define the maximum range of short integers
#define SHORT_INT_MAX SHRT_MAX

// Hash function to map long integers to short integers
short map_long_to_short(long value) {
    if (value < 0 || value > LONG_INT_MAX) {
        return -1; // Or some other error code
    }
    return (short)(value % SHORT_INT_MAX);
}

int compare_at(const char *str1,const char *str2,int pos){
        int i;
        for (i=0;i<pos && str1[i]!='\0' && str2[i]!='\0';i++){
                if (str1[i]!=str2[i]){
                        return 0;
                }
        }
        return 1;
}

void cut_at(int x,int y,int width,int height){
    y = (y+y_buffer)* pixel_ratio;
    x = (x+x_buffer)* pixel_ratio;
    for (int rel_y=y;rel_y-y<pixel_ratio;rel_y++){
            for (int rel_x=x;rel_x-x<pixel_ratio;rel_x++){
                    total_image[rel_y][rel_x].r = 0;
                    total_image[rel_y][rel_x].g = 0;
                    total_image[rel_y][rel_x].b = 0;
                    total_image[rel_y][rel_x].a = 0;
            }
    }
}
void draw_at(int x,int y,int width,int height,int r,int g,int b){
        y = (y+y_buffer)*pixel_ratio;
        x = (x+x_buffer)*pixel_ratio;
        for (int rel_y=y;rel_y-y<pixel_ratio;rel_y++){
                for (int rel_x=x;rel_x-x<pixel_ratio;rel_x++){
                        total_image[rel_y][rel_x].r = r;
                        total_image[rel_y][rel_x].g = g;
                        total_image[rel_y][rel_x].b = b;
                        total_image[rel_y][rel_x].a = 255;
                }
        }
}
int xy_point_in(xy_point *points,int point_num,int x,int y){
        for (int i=0;i<point_num;i++){
                if (points[i].x == x && points[i].y == y){
                        return 1;
                }
        }
        return 0;
}

int in_range(int start,int end,int value){
        if (value >= start && value <= end){
                return 1;
        }
        return 0;
}

int colors_contains(RGB_spectrum color){
        for (int i=0;i<current_colors;i++){
                if (colors[i].r == color.r && colors[i].g == color.g && colors[i].b == color.b){
                        return i;
                }
        }
        return -1;
}
