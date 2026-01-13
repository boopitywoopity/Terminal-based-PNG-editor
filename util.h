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
} point_w_colour;

typedef struct {
    char *fname;
    // this will determine how many times an operation will loop, ie if you do "h" to move left it will determine how many times it will move to the left
    int loop_count;
    bool loop_count_assigned;

    uint32_t user_colours;
    int current_colour;

    enum traverse_mode {
        IMAGE_TRAVERSE = 1,
        INDIVIDUAL_SELECT = 2,
        BOX_SELECT = 3
    } mode;
} program_info;

typedef struct tree_t {
    uint32_t height;
    uint64_t colour_code;
    uint8_t colour[3];
    uint32_t colour_32;
    struct tree_t *l, *r;
} tree;

// This struct isused to facilitate passing information between functions.
// It should contain most of everything the function needs in terms of image details.
typedef struct {
    unsigned int y_buffer, x_buffer;
    unsigned int pixel_ratio;

    unsigned int width, height;

    uint32_t **total_image;

    unsigned int colour_index, current_colours, current_max_colours;
    // RGB_spectrum *colours;
    tree *root;

    xy_point *points;
} image_container;

// util.c

int compare_at(const char *str1, const char *str2, int pos);

void cut_at(image_container *img, unsigned int x, unsigned int y);

void draw_at(image_container *img, int x, int y, unsigned int r, unsigned int g, unsigned int b);

int xy_point_in(xy_point *points, int point_count, int x, int y);

int in_range(int start, int end, int value);

int colours_contains(image_container *img, unsigned int r, unsigned int g, unsigned int b);

// image-loader.c

image_container load_image(unsigned char *image, unsigned int width, unsigned int height);

program_info generate_default_program_info(char *fname);



// avl.c

bool contains(tree **root, const uint32_t target);
uint64_t get_colour_code(tree **root, const uint32_t target);
void insert(tree **root, const uint32_t value);

// keyboard-input.c

void key_press(image_container *img, program_info *info, unsigned int *x, unsigned int *y, char input, int *quit);

// TODO: figure out where this should go

void select_from_image(image_container *img, program_info *info, unsigned int *x, unsigned int *y);
void load_colours_select(image_container *img, xy_point *points, int point_num);

void load_colours_select_fill(image_container *img, int init_y, int init_x, int range_y, int range_x);
void select_fill_from_image(image_container *img, program_info *info, unsigned int *x, unsigned int *y);

void load_image_information(image_container *img, program_info *info, int x, int y);
void generate_colours_from_pixel_ratio(image_container *img, unsigned int *y, unsigned int *x);

WINDOW *create_window(unsigned int height, unsigned int width);
int load_colours(image_container *img);
#endif // UTIL_H
