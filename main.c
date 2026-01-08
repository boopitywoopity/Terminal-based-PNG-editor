#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "util.h"

// TODO
// 1. Refactor main such that it conforms with the standard
// 2. Refactor the key_press function such that it's not a piece of shit
// 3. Implement regex to the key_press for checking user input
// 4. Fix potential memory leaks
// 5. Spend time understanding what some parts of the code do and how/why (been a couple years since I originally wrote this some parts of this are weird)
// 6. Rewrite the way colors are done, each point are a hash that points to a value in a tree (avl tree?)



// example of how to make colour pairs in case I forget...
/* init_extended_color(pair_index,r*1000/255,g*1000/255,b*1000/255); */
/* init_extended_pair(pair_index,pair_index,COLOR_BLACK); */
// compile with flag --enable-ext-colours

int main(int e, char **args) {
    if (e == 1) {
        printf("There was no file provided\n");
        return 0;
    }

    create_window(LINES, COLS);

    // verify compatibility
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support colours\n");
        return 0;
    }

#ifdef NCURSES_VERSION
#if NCURSES_VERSION_MAJOR >= 6 && NCURSES_VERSION_MINOR >= 1
    if (can_change_color() && COLORS >= 32767) {
        printf("Extended colour support enabled!\n");
    }
#endif
#endif

    if (can_change_color() == FALSE) {
        endwin();
        printf("Your terminal does not support changing colours\n");
        return 0;
    }

    program_info info = generate_default_program_info(args[1]);

    int width, height, channels;
    unsigned char *image = stbi_load(args[1], &width, &height, &channels, 4);

    if (!image) {
        printf("There was an issue opening the image\n");
        return 0;
    }

    image_container img = load_image(image, width, height);
    load_colours(&img);

    unsigned int curx = 0;
    unsigned int cury = 0;

    curs_set(2);

    char ch; // the input provided by the user
    int quit = 0;

    // this the main loop (no shit sherlock)
    while (!quit) {
        load_image_information(&img, &info, curx, cury);
        move(cury, curx);
        ch = getch();
        key_press(&img, &info, &curx, &cury, ch, &quit);
    }



    // cleanup
    endwin();
    stbi_image_free(image); // the following lines of code just free items
    for (int i = 0; i < img.current_colours; i++) {
        free(img.colours[i].point);
    }

    free(info.user_colours);
    free(img.colours);
    free(img.total_image);

    printf("Colour count at program stop: %d\n", img.current_colours);
    printf("'Colours' size at program stop: %d\n", img.current_max_colours);
    return 0;
}

void generate_colours_from_pixel_ratio(image_container *img, unsigned int *y, unsigned int *x) {
    if (*y + img->y_buffer > img->height) {
        img->y_buffer = img->height - *y;
        if (img->y_buffer < 0) {
            img->y_buffer = 0;
            *y = img->height;
        }
    }
    if (*x + img->x_buffer > img->width) {
        img->x_buffer = img->width - *x;
        if (img->x_buffer < 0) {
            img->x_buffer = 0;
            *x = img->width;
        }
    }
    int y_pos = 0;
    int x_pos = 0;
    for (int i = 0; i < img->current_colours; i++) {
        free(img->colours[i].point);
    }

    free(img->colours);
    img->colours = malloc(sizeof(RGB_spectrum) * (img->current_max_colours));
    img->colour_index = START_COLOR_INDEX;
    img->current_colours = 0;
    for (int y = 0; y < img->height; y += img->pixel_ratio) {
        x_pos = 0;
        for (int x = 0; x < img->width; x += img->pixel_ratio) {
            int r_total = 0;
            int g_total = 0;
            int b_total = 0;
            int a_total = 0;
            for (int rel_y = y; rel_y - y < img->pixel_ratio; rel_y++) {
                for (int rel_x = x; rel_x - x < img->pixel_ratio; rel_x++) {
                    r_total += img->total_image[rel_y][rel_x].r;
                    g_total += img->total_image[rel_y][rel_x].g;
                    b_total += img->total_image[rel_y][rel_x].b;
                    a_total += img->total_image[rel_y][rel_x].a;
                }
            }

            if (a_total != 0) {
                unsigned int p2 = img->pixel_ratio * img->pixel_ratio;
                unsigned int r = r_total / p2;
                unsigned int g = g_total / p2;
                unsigned int b = b_total / p2;

                int colour_position = colours_contains(img, r, g, b);
                if (colour_position != -1) {
                    unsigned int *max_length = &img->colours[colour_position].max_length;
                    unsigned int *length = &img->colours[colour_position].length;
                    if (*length == *max_length){
                        (*max_length) *= 2;
                        img->colours[colour_position].point = realloc(img->colours[colour_position].point, sizeof(xy_point)*(*max_length));
                        if (img->colours[colour_position].point == NULL){
                            fprintf(stderr, "Error: failure to reallocate memory for the point\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    img->colours[colour_position].point[*length].x = x_pos;
                    img->colours[colour_position].point[*length].y = y_pos;
                    img->colours[colour_position].length += 1;
                }
                else {
                    RGB_spectrum colour;
                    colour.r = r;
                    colour.g = g;
                    colour.b = b;
                    // TODO Fix this part of the code such that it doesnt use arbitrary size declaration
                    colour.max_length = MAX_STARTING_POINT_COUNT;
                    colour.point = malloc(sizeof(xy_point) * colour.max_length);
                    colour.point[0].x = x_pos;
                    colour.point[0].y = y_pos;
                    colour.length = 1;
                    img->colours[img->current_colours] = colour;
                    img->colours[img->current_colours].code = img->colour_index;
                    img->current_colours += 1;
                }
            }
            else {
                // What the fuck is this?
            }
            x_pos += 1;
        }
        y_pos += 1;
    }
}

void select_from_image(image_container *img, program_info *info, unsigned int *x, unsigned int *y) {
    int max_coords = 256; // the maximum ammount of coords currently selected, used to grow the coords if necessary
    xy_point *coord = malloc(sizeof(xy_point) * max_coords); // all the selected coords
    // set the initial coord
    coord[0].x = *x + img->x_buffer;
    coord[0].y = *y + img->y_buffer;
    int coord_count = 1;

    load_colours_select(img, coord, coord_count);

    char input;
    int run = 1;

    while (run) {
        move(*y, *x);
        input = getch();
        if ('0' <= input && input <= '9') {
            if (!info->loop_count_assigned) {
                info->loop_count = input - '0';

            }
            else {
                info->loop_count = info->loop_count * 10;
                info->loop_count += input - '0';
            }
        }
        switch (input) {
            case 27: // escape, stop selecting from the image
                if (info->loop_count_assigned) {
                    info->loop_count = 1;
                    info->loop_count_assigned = false;
                }
                else {
                    run = 0;
                }
                break;
            case 'h':
                for (int i = 0; i < info->loop_count; i++) {
                    if (*x > 0) {
                        *x -= 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                    else if (img->x_buffer > 0) {
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        img->x_buffer -= 1;
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(img, coord, coord_count);
                info->loop_count = 1;
                info->loop_count_assigned = false;
                break;
            case 'l':
                for (int i = 0; i < info->loop_count && i < img->height / img->pixel_ratio; i++) {
                    if (*x + 1 < img->width / img->pixel_ratio && *x < COLS - 2) {
                        *x += 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                    else if (img->x_buffer + *x + 1 < img->width / img->pixel_ratio && *x == COLS - 2) {
                        img->x_buffer += 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(img, coord, coord_count);
                info->loop_count = 1;
                info->loop_count_assigned = false;
                break;
            case 'j':
                for (int i = 0; i < info->loop_count && i < img->height / img->pixel_ratio; i++) {
                    if (*y + 1 < img->height / img->pixel_ratio && *y < LINES - 3) {
                        *y += 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                    else if (img->y_buffer + *y + 1 < img->height / img->pixel_ratio &&
                        *y == LINES - 3) {
                        img->y_buffer += 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(img, coord, coord_count);
                info->loop_count = 1;
                info->loop_count_assigned = false;
                break;
            case 'k':
                for (int i = 0; i < info->loop_count; i++) {
                    if (*y > 0) {
                        *y -= 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                    else if (img->y_buffer > 0) {
                        img->y_buffer -= 1;
                        if (coord_count == max_coords){
                            max_coords *= 2;
                            coord = realloc(coord, sizeof(xy_point)*max_coords);
                        }
                        coord[coord_count].x = *x + img->x_buffer;
                        coord[coord_count].y = *y + img->y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(img, coord, coord_count);
                info->loop_count = 1;
                info->loop_count_assigned = false;
                break;
            case 'c':
                for (int i = 0; i < coord_count; i++) {
                    cut_at(img, coord[i].x - img->x_buffer, coord[i].y - img->y_buffer);
                }
                info->loop_count = 1;
                info->loop_count_assigned = false;
                run = 0;
                break;
            case 'd':
                for (int i = 0; i < coord_count && info->current_colour != -1; i++) {
                    int x = coord[i].x;
                    int y = coord[i].y;
                    draw_at(img, x - img->x_buffer, y - img->y_buffer,
                            info->user_colours[info->current_colour].r, info->user_colours[info->current_colour].g,
                            info->user_colours[info->current_colour].b);
                }
                info->loop_count_assigned = false;
                info->loop_count = 0;
                run = 0;
                break;
        }
    }
    free(coord);
}

void load_colours_select(image_container *img, xy_point *points, int point_num) {

    move(0, 0);
    attron(A_NORMAL);
    for (int y = 0; y < LINES - 2; y++) {
        for (int x = 0; x < COLS - 1; x++) {
            mvaddch(y, x, 'X');
        }
    }
    attroff(A_NORMAL);
    for (int i = 0; i < img->current_colours; i++) {
        img->colours[i].code = img->colour_index;
        init_extended_color(img->colour_index, img->colours[i].r * 1000 / 255, img->colours[i].g * 1000 / 255, img->colours[i].b * 1000 / 255);
        init_extended_pair(img->colour_index, COLOR_BLACK, img->colour_index);
        attron(COLOR_PAIR(img->colour_index));
        for (int n = 0; n < img->colours[i].length; n++) {
            int cur_y = img->colours[i].point[n].y;
            int cur_x = img->colours[i].point[n].x;
            if (cur_x - img->x_buffer < COLS - 1 && cur_y - img->y_buffer < LINES - 2) {
                if (xy_point_in(points, point_num, cur_x, cur_y)) {
                    mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, '0');

                } else {
                    mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, ' ');
                }
            }
        }
        attroff(COLOR_PAIR(img->colour_index));
        img->colour_index += 1;
    }
    img->colour_index = START_COLOR_INDEX;
    refresh();
}
void select_fill_from_image(image_container *img, program_info *info, unsigned int *x, unsigned int *y) {
    int init_y = *y + img->y_buffer;
    int init_x = *x + img->x_buffer;

    int range_y = 0;
    int range_x = 0;
    int run = 1;
    char input;

    load_colours_select_fill(img, init_y, init_x, range_y, range_x);

    while (run) {
        move(*y, *x);
        input = getch();
        if (input >= '0' && input <= '9') {
            if (!info->loop_count_assigned) {
                info->loop_count = input - '0';
                info->loop_count_assigned = true;
            }
            else {
                info->loop_count *= 10;
                info->loop_count += input - '0';
            }
        }
        load_image_information(img, info, *x, *y);
        switch (input) {
            case 27:
                if (info->loop_count_assigned) {
                    info->loop_count_assigned = false;
                    info->loop_count = 1;
                } else {
                    run = 0;
                }
                break;
            case 'h':
                for (int i = 0; i < info->loop_count; i++) {
                    if (*x != 0) {
                        *x -= 1;
                        range_x -= 1;
                    } else if (img->x_buffer > 0) {
                        img->x_buffer -= 1;
                        range_x -= 1;
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                load_colours_select_fill(img, init_y, init_x, range_y, range_x);
                break;
            case 'j':
                for (int i = 0; i < info->loop_count && i < img->height; i++) {
                    if (*y + 1 < img->height / img->pixel_ratio && *y < LINES - 3) {
                        *y += 1;
                        range_y += 1;
                    }
                    else if (img->y_buffer + *y + 1 < img->height / img->pixel_ratio && *y == LINES - 3) {
                        img->y_buffer += 1;
                        range_y += 1;
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                load_colours_select_fill(img, init_y, init_x, range_y, range_x);
                break;
            case 'k':
                for (int i = 0; i < info->loop_count; i++) {
                    if (*y != 0) {
                        *y -= 1;
                        range_y -= 1;
                    }
                    else if (img->y_buffer > 0) {
                        img->y_buffer -= 1;
                        range_y -= 1;
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                load_colours_select_fill(img, init_y, init_x, range_y, range_x);
                break;
            case 'l':
                for (int i = 0; i < info->loop_count && i < img->width; i++) {
                    if (*x + 1 < img->width / img->pixel_ratio && *x < COLS - 2) {
                        *x += 1;
                        range_x += 1;
                    }
                    else if (img->x_buffer + *x + 1 < img->width / img->pixel_ratio && *x == COLS - 2) {
                        img->x_buffer += 1;
                        range_x += 1;
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                load_colours_select_fill(img, init_y, init_x, range_y, range_x);
                break;
            case 'c':
                if (true) {
                } // REMOVE WHEN ITS DONE
                int tmp_init_y = init_y;
                int tmp_init_x = init_x;
                int y_range = init_y + range_y;
                int x_range = init_x + range_x;
                if (y_range < tmp_init_y) {
                    int tmp_y = tmp_init_y;
                    tmp_init_y = y_range;
                    y_range = tmp_y;
                }
                if (range_x < 0) {
                    int tmp_x = tmp_init_x;
                    tmp_init_x = x_range;
                    x_range = tmp_x;
                }
                for (int y_pos = tmp_init_y; y_pos <= y_range; y_pos++) {
                    for (int x_pos = tmp_init_x; x_pos <= x_range; x_pos++) {
                        cut_at(img, x_pos - img->x_buffer, y_pos - img->y_buffer);
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                run = 0;
                break;
            case 'd':
                if (true) {
                }
                int tmp_init_y1 = init_y;
                int tmp_init_x1 = init_x;
                int y_range1 = init_y + range_y;
                int x_range1 = init_x + range_x;
                if (y_range1 < tmp_init_y1) {
                    int tmp_y = tmp_init_y1;
                    tmp_init_y1 = y_range1;
                    y_range1 = tmp_y;
                }
                if (range_x < 0) {
                    int tmp_x = tmp_init_x1;
                    tmp_init_x1 = x_range1;
                    x_range1 = tmp_x;
                }
                for (int y_pos = tmp_init_y1; y_pos <= y_range1 && info->current_colour != -1; y_pos++) {
                    for (int x_pos = tmp_init_x1; x_pos <= x_range1; x_pos++) {
                        draw_at(img, x_pos - img->x_buffer, y_pos - img->y_buffer,
                                info->user_colours[info->current_colour].r,
                                info->user_colours[info->current_colour].g,
                                info->user_colours[info->current_colour].b
                                );
                    }
                }
                info->loop_count_assigned = false;
                info->loop_count = 1;
                run = 0;
                break;
        }
    }
}

void load_colours_select_fill(image_container *img, int init_y, int init_x, int range_y, int range_x) {
    move(0, 0);
    attron(A_NORMAL);
    for (int y = 0; y < LINES - 2; y++) {
        for (int x = 0; x < COLS - 1; x++) {
            mvaddch(y, x, 'X');
        }
    }
    attroff(A_NORMAL);

    int y_range = init_y + range_y;
    int x_range = init_x + range_x;
    if (y_range < init_y) {
        int tmp_y = init_y;
        init_y = y_range;
        y_range = tmp_y;
    }
    if (range_x < 0) {
        int tmp_x = init_x;
        init_x = x_range;
        x_range = tmp_x;
    }
    for (int i = 0; i < img->current_colours; i++) {
        img->colours[i].code = img->colour_index;
        init_extended_color(img->colour_index, img->colours[i].r * 1000 / 255, img->colours[i].g * 1000 / 255, img->colours[i].b * 1000 / 255);
        init_extended_pair(img->colour_index, COLOR_BLACK, img->colour_index);
        attron(COLOR_PAIR(img->colour_index));
        for (int n = 0; n < img->colours[i].length; n++) {
            int cur_y = img->colours[i].point[n].y;
            int cur_x = img->colours[i].point[n].x;
            if ((cur_y >= img->y_buffer && cur_y < LINES - 2 + img->y_buffer) &&
                (cur_x >= img->x_buffer && cur_x < COLS - 1 + img->x_buffer) &&
                cur_x - img->x_buffer < COLS - 1 && cur_y - img->y_buffer < LINES - 2) {
                if (in_range(init_y, y_range, cur_y) &&
                    in_range(init_x, x_range, cur_x)) {
                    mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, '0');
                } else {
                    mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, ' ');
                }
            }
        }
        attroff(COLOR_PAIR(img->colour_index));
        img->colour_index += 1;
    }
    img->colour_index = START_COLOR_INDEX;
    refresh();
}

// Load the entire image
int load_colours(image_container *img) {
    // TODO: rewrite this such that it's not fucking stupid
    move(0, 0);
    attron(A_NORMAL);
    for (int y = 0; y < LINES - 2; y++) {
        for (int x = 0; x < COLS - 1; x++) {
            mvaddch(y, x, 'X');
        }
    }
    attroff(A_NORMAL);
    for (int i = 0; i < img->current_colours; i++) {
        img->colours[i].code = img->colour_index;
        init_extended_color(img->colour_index, img->colours[i].r * 1000 / 255, img->colours[i].g * 1000 / 255, img->colours[i].b * 1000 / 255);
        init_extended_pair(img->colour_index, COLOR_BLACK, img->colour_index);
        attron(COLOR_PAIR(img->colour_index));
        for (int n = 0; n < img->colours[i].length; n++) {
            int cur_y = img->colours[i].point[n].y;
            int cur_x = img->colours[i].point[n].x;
            if ((cur_y >= img->y_buffer && cur_y - img->y_buffer < LINES - 2)
                && (cur_x >= img->x_buffer && cur_x - img->x_buffer < COLS - 1)){
                mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, ' ');
            }
        }
        attroff(COLOR_PAIR(img->colour_index));
        img->colour_index += 1;
    }
    img->colour_index = START_COLOR_INDEX;
    refresh();
    return 0;
}

void load_image_information(image_container *img, program_info *info, int x, int y) {
    move(LINES - 2, 0);
    clrtoeol();
    mvprintw(LINES - 2, 10, "%s             %d:%d", info->fname, img->height, img->width);
    mvprintw(LINES - 2, COLS - 20, "(%d,%d)     %d", y + img->y_buffer, x + img->x_buffer, info->loop_count);
}

WINDOW *create_window(unsigned int height, unsigned int width) {
    initscr();
    WINDOW *win = newwin(width, height, 0, 0);
    keypad(win, true);
    start_color();
    noecho();
    cbreak();
    curs_set(0);

    return win;
}
