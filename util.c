#include <stdlib.h>
#include <ncurses.h>
#define MAX_COLOURS 100000
#define START_COLOR_INDEX 27

struct xy_point {
        int x;
        int y;
};
struct RGB_spectrum {
        unsigned int r;
        unsigned int g;
        unsigned int b;
        struct xy_point *point;
        unsigned int length;
        unsigned int code;
};

struct point_w_color{
        unsigned int r;
        unsigned int g;
        unsigned int b;
        unsigned int a;
};

// variables
WINDOW *win;
int x_buffer = 0;
int y_buffer = 0;
int loop_count = 1;
int loop_input_assigned = 0;

struct RGB_spectrum *colours;
int current_colours = 0;
int colour_index = START_COLOR_INDEX;

int pixel_ratio = 1;
struct point_w_color **total_image;

int current_colour = -1;
struct point_w_color *user_colours;


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
int xy_point_in(struct xy_point *points,int point_num,int x,int y){
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

int colours_contains(struct RGB_spectrum colour){
        for (int i=0;i<current_colours;i++){
                if (colours[i].r == colour.r && colours[i].g == colour.g && colours[i].b == colour.b){
                        return i;
                }
        }
        return -1;
}
