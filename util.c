#include <ncurses.h>

#include "util.h"

// define a maximum possible point count for initial point creation
#define MAX_STARTING_POINT_COUNT 128

#include <limits.h>
// Define the maximum range of your long integers
#define LONG_INT_MAX 1000000

// Define the maximum range of short integers
#define SHORT_INT_MAX SHRT_MAX

int compare_at(const char *str1,const char *str2,int pos){
    int i;
    for (i=0;i<pos && str1[i]!='\0' && str2[i]!='\0';i++){
        if (str1[i]!=str2[i]){
            return 0;
        }
    }
    return 1;
}

// void cut_at(image_container *img, unsigned int x,unsigned int y){
//     const unsigned int y_buffer = img->y_buffer;
//     const unsigned int x_buffer = img->x_buffer;
//     const unsigned int pixel_ratio = img->pixel_ratio;
//
//     const unsigned int width = img->width;
//     const unsigned int height = img->height;
//
//     point_w_colour **total_image = img->total_image;
//
//     y = (y+y_buffer)* pixel_ratio;
//     x = (x+x_buffer)* pixel_ratio;
//     for (int rel_y=y;rel_y-y<pixel_ratio;rel_y++){
//         for (int rel_x=x;rel_x-x<pixel_ratio;rel_x++){
//             total_image[rel_y][rel_x].r = 0;
//             total_image[rel_y][rel_x].g = 0;
//             total_image[rel_y][rel_x].b = 0;
//             total_image[rel_y][rel_x].a = 0;
//         }
//     }
// }

// sets a selected area of the image to the color seleced by the user
// void draw_at(image_container *img, int x,int y, unsigned int r, unsigned int g, unsigned int b){
//     const unsigned int y_buffer = img->y_buffer;
//     const unsigned int x_buffer = img->x_buffer;
//     const unsigned int pixel_ratio = img->pixel_ratio;
//
//     const unsigned int width = img->width;
//     const unsigned int height = img->height;
//
//     point_w_colour **total_image = img->total_image;
//     y = (y+y_buffer)*pixel_ratio;
//     x = (x+x_buffer)*pixel_ratio;
//     for (int rel_y=y;rel_y-y<pixel_ratio;rel_y++){
//         for (int rel_x=x;rel_x-x<pixel_ratio;rel_x++){
//             total_image[rel_y][rel_x].r = r;
//             total_image[rel_y][rel_x].g = g;
//             total_image[rel_y][rel_x].b = b;
//             total_image[rel_y][rel_x].a = 255;
//         }
//     }
// }

// this serves to check if a point is in the selected points
int xy_point_in(xy_point *selected_points,int point_count,int x,int y){
    for (int i=0;i<point_count;i++){
        if (selected_points[i].x == x && selected_points[i].y == y){
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

// this verifies if a colour already exists
// int colours_contains(image_container *img, const unsigned int r, const unsigned int g, const unsigned int b){
//     const unsigned int current_colours = img->current_colours;
//     const RGB_spectrum *colours = img->colours;
//
//     for (int i=0;i<current_colours;i++){
//         if (colours[i].r == r && colours[i].g == g && colours[i].b == b){
//             return i;
//         }
//     }
//     return -1;
// }
