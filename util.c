#include <ncurses.h>

#include "util.h"
#include "globals.h"

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

// sets a selected area of the image to the color seleced by the user
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
