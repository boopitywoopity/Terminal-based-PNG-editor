#ifndef UTIL_H
#define UTIL_H
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
} RGB_spectrum;

typedef struct {
        unsigned int r;
        unsigned int g;
        unsigned int b;
        unsigned int a;
} point_w_color;

// util.c

int compare_at(const char *str1, const char *str2, int pos);

void cut_at(int x, int y, int width, int height);

void draw_at(int x, int y, int width, int height, int r, int g, int b);

int xy_point_in(xy_point *point, int point_num, int x, int y);

int in_range(int start, int end, int value);

int colors_contains(RGB_spectrum color);

// image-loader.c

void load_image(unsigned char *image, int width, int height);
#endif // UTIL_H
