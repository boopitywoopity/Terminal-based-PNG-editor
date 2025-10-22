#include "util.c"
#include <curses.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

void select_from_image(unsigned int *x, unsigned int *y, int width, int height);
void load_colours_select( xy_point *points, int point_num, int width,
                         int height);

void load_colours_select_fill(int init_y, int init_x, int range_y, int range_x,
                              int width, int height);
void select_fill_from_image(unsigned int *x, unsigned int *y, int width,
                            int height);

void load_image_information(int x, int y, int height, int width);
void generate_colors_from_pixel_ratio(unsigned int *y, unsigned int *x,
                                      int height, int width);
void key_press(unsigned int *x, unsigned int *y, int *width, int *height,
               char input, int *quit);
void create_window(int height, int width);
int load_colours(int width, int height);

// example of how to make colour pairs in case I forget...
/* init_extended_color(pair_index,r*1000/255,g*1000/255,b*1000/255); */
/* init_extended_pair(pair_index,pair_index,COLOR_BLACK); */
// compile with flag --enable-ext-colors

const char *filename;

int main(int e, char **args) {
    if (e == 1) {
        printf("There was no file provided\n");
        return 0;
    }

    filename = args[1];
    int width, height, channels;
    unsigned char *image = stbi_load(filename, &width, &height, &channels, 4);

    fprintf(stderr, "channels: %d\n", channels);
    fprintf(stderr, "width: %d\nheight: %d\n", width, height);

    if (!image) {
        printf("There was an issue opening the image\n");
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
        printf("Extended color support enabled!\n");
    }
#endif
#endif

    if (can_change_color() == FALSE) {
        endwin();
        printf("Your terminal does not support changing colours\n");
        return 0;
    }
    // fprintf(stderr, "test 0\n");

    // these are user defined colours for drawing
    // the user can only define 10 colors (for now might change later)
    user_colours = malloc(sizeof( point_w_color) * 10);
    for (int user_clr_pos = 0; user_clr_pos < 10; user_clr_pos++) { // define them all as black by default
        user_colours[user_clr_pos].r = 0;
        user_colours[user_clr_pos].g = 0;
        user_colours[user_clr_pos].b = 0;
    }
    // fprintf(stderr, "test 1\n");

    colours = malloc(sizeof(RGB_spectrum) * current_max_colours);

    total_image = malloc(sizeof(point_w_color *) * (height+1));
    if (total_image == NULL) { // there was a failur to allocate enough memory for the total image
        endwin();
        printf("There was an issue allocating memory for the total image\n");
        return 0;
    }

    // fprintf(stderr, "test 2\n");

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
                // fprintf(stderr, "current_colours: %d\n", current_colours);

                // create a new color, if the color already exists just ignore this and use the existing one, otherwise use it
                // a better way to do this might be to hash the rgb values and then look to see if it exists
                RGB_spectrum colour;
                colour.r = image[index];
                colour.g = image[index + 1];
                colour.b = image[index + 2];
                colour.length = 0;

                // fprintf(stderr, "colour: (%d, %d, %d) ", colour.r, colour.g, colour.b);

                // this color already exists, bind the point to the color
                int colour_position = colours_contains(colour);
                if (colour_position != -1) {
                    // create a new point and add it to an existing color
                    xy_point point;
                    point.x = x;
                    point.y = y;
                    // this sets the
                    colours[colour_position].point[colours[colour_position].length++] = point;
                    // fprintf(stderr, "exists at index: %d\n", colour_position);
                }
                // this color does not exist bind the new color to the colors and then bind the current location to the color
                else {

                    // fprintf(stderr, "does not exist\n");

                    if (current_colours+1 == INT_MAX){ // you have too many colours, kill the program
                        endwin();
                        stbi_image_free(image);
                        // Free all the colours
                        for (int i = 0; i < current_colours; i++) {
                            free(colours[i].point);
                        }
                        free(colours);
                        free(total_image);
                        printf("The image you are attempting to load has esceeded the maximum ammount of colours available\n");
                        return 0;
                    }

                    if (current_colours + 1 == current_max_colours) { // you have exceeded the maximum ammount of colors that is currently allocated, resize the array
                        // fprintf(stderr, "resize needed, current size: %d, new current size: %d\n", current_max_colours, current_max_colours*2);
                        current_max_colours *= 2;
                        colours = realloc(colours, sizeof(RGB_spectrum)*current_max_colours);
                    }

                    // create another point
                    xy_point point;
                    point.x = x;
                    point.y = y;

                    colour.point = malloc(sizeof( xy_point *) * 1000000000);
                    colour.point[colour.length++] = point;
                    colours[current_colours++] = colour;

                }
            }
        }
    }

    load_colours(width, height);
    unsigned int curx = 0;
    unsigned int cury = 0;
    curs_set(2);
    char ch;
    int quit = 0;

    // this the main loop
    while (!quit) {
        load_image_information(curx, cury, height, width);
        move(cury, curx);
        ch = getch();
        key_press(&curx, &cury, &width, &height, ch, &quit);
    }

    endwin();
    stbi_image_free(image); // the following lines of code just free items
    for (int i = 0; i < current_colours; i++) {
        free(colours[i].point);
    }

    free(user_colours);
    free(colours);
    free(total_image);

    printf("Colour count at program stop: %d\n", current_colours);
    printf("'Colours' size at program stop: %d\n", current_max_colours);
    return 0;
}
void key_press(unsigned int *x, unsigned int *y, int *width, int *height,
               char input, int *quit) {
    if (input >= '0' && input <= '9') {
        if (!loop_input_assigned) {
            loop_count = input - '0';
            loop_input_assigned = 1;
        } else {
            loop_count *= 10;
            loop_count += input - '0';
        }
        return;
    }
    switch (input) {
        case 'h':
            for (int i = 0; i < loop_count; i++) {
                if (*x != 0) {
                    *x -= 1;
                } else if (x_buffer > 0) {
                    x_buffer -= 1;
                }
            }
            load_colours(*width, *height);
            break;
        case 'j':
            for (int i = 0; i < loop_count && i < *height / pixel_ratio; i++) {
                if (*y + 1 < *height / pixel_ratio && *y < LINES - 3) {
                    *y += 1;
                } else if (y_buffer + *y + 1 < *height / pixel_ratio && *y == LINES - 3) {
                    y_buffer += 1;
                }
            }
            load_colours(*width, *height);
            break;
        case 'k':
            for (int i = 0; i < loop_count; i++) {
                if (*y != 0) {
                    *y -= 1;
                } else if (y_buffer > 0) {
                    y_buffer -= 1;
                }
            }
            load_colours(*width, *height);
            break;
        case 'l':
            for (int i = 0; i < loop_count && i < *width / pixel_ratio; i++) {
                if (*x + 1 < *width / pixel_ratio && *x < COLS - 2) {
                    *x += 1;
                } else if (x_buffer + *x + 1 < *width / pixel_ratio && *x == COLS - 2) {
                    x_buffer += 1;
                }
            }
            load_colours(*width, *height);
            break;
        case 'd':
            if (current_colour == -1)
                break;
            draw_at(*x, *y, *width, *height, user_colours[current_colour].r,
                    user_colours[current_colour].g, user_colours[current_colour].b);
            generate_colors_from_pixel_ratio(&*y, &*x, *height, *width);
            load_colours(*width, *height);
            break;
        case 'c':
            cut_at(*x, *y, *width, *height);
            generate_colors_from_pixel_ratio(&*y, &*x, *height, *width);
            load_colours(*width, *height);
            break;
        case 'v':
            select_fill_from_image(&*x, &*y, *width, *height);
            generate_colors_from_pixel_ratio(&*y, &*x, *height, *width);
            load_colours(*width, *height);
            break;
        case 'V':
            select_from_image(&*x, &*y, *width, *height);
            generate_colors_from_pixel_ratio(&*y, &*x, *height, *width);
            load_colours(*width, *height);
            break;
        case ':': // handles commands from the user
            // REMOVE WHEN ITS FINISHED
            if (true)
                ; // this is there because if it isnt the lsp screams at me that there is
            // an error, but it still runs...
            int command_len = 0;
            char *command = malloc(sizeof(char) * 100);
            char input;
            int xpos = 1;
            int value = 0;
            mvaddch(LINES - 1, 0, ':');

            // get the full command from the user, then compare it to see if it matches
            while ((input = getch()) != '\n') { // get the command input from the user
                if (input == 127 && xpos > 1) {
                    command[command_len--] = '\0';
                    mvaddch(LINES - 1, --xpos, ' ');
                    move(LINES - 1, xpos);
                } else if (input != 127) {
                    value = value * 10 + input - '0';
                    command[command_len++] = input;
                    mvaddch(LINES - 1, xpos++, input);
                }
            }

            if (compare_at(command, "q", 1)) {
                *quit = 1;
            } else if (compare_at(command, "col", 3)) {
                for (int i = LINES - 3; i < LINES - 1; i++) {
                    move(i, 0);
                    clrtoeol();
                }
                for (int i = 0; i < 10; i++) { // generates the colors for the user to
                    // view the colour templates
                    float colour_brightness =
                        (user_colours[i].r * 0.299 + 0.587 * user_colours[i].g +
                        0.144 * user_colours[i].b) /
                        255;
                    init_extended_color(17 + i, user_colours[i].r * 1000 / 255,
                               user_colours[i].g * 1000 / 255,
                               user_colours[i].b * 1000 / 255);
                    if (colour_brightness < 0.5) {
                        init_extended_pair(17 + i, COLOR_WHITE, 17 + i);
                    } else {
                        init_extended_pair(17 + i, COLOR_BLACK, 17 + i);
                    }
                    attron(COLOR_PAIR(17 + i));
                    mvprintw(LINES - 3, i * 10, "Colour %d", i);
                    attroff(COLOR_PAIR(17 + i));
                }
                move(LINES - 1, 0);
                clrtoeol();
                char *colour_command = malloc(sizeof(char) * 100);
                int col_cmd_len = 0;
                char col_input;
                int cmd_selected_color = -1; // this is the selected color that the user
                // has chosen for the action

                while ((col_input = getch()) != '\n' && col_cmd_len != 100 &&
                    col_input != 27) {
                    if (cmd_selected_color == -1 && col_input >= '0' && col_input <= '9') {
                        cmd_selected_color = col_input - '0';
                        addch(col_input);
                    } else if (cmd_selected_color != -1) {
                        if (col_input == 127) {
                            if (col_cmd_len == 0) {
                                cmd_selected_color = -1;
                                mvaddch(LINES - 1, 0, ' ');
                                move(LINES - 1, 0);
                            } else {
                                colour_command[col_cmd_len--] = '\0';
                                mvaddch(LINES - 1, col_cmd_len, ' ');
                                move(LINES - 1, col_cmd_len);
                            }
                        } else {
                            colour_command[col_cmd_len++] = col_input;
                            addch(col_input);
                        }
                    }
                }
                if (colour_command[col_cmd_len] != '\0') {
                    colour_command[col_cmd_len] = '\0';
                }
                if (col_input != 27 && cmd_selected_color != -1) {
                    if (strcmp(colour_command, "-use") == 0) {
                        current_colour = cmd_selected_color;
                    } else if (strcmp(colour_command, "-set") == 0) {
                        int accept = 0;
                        while (!accept) {
                            int r = 0;
                            int g = 0;
                            int b = 0;

                            int cur = 0;
                            char num;
                            move(LINES - 1, 0);
                            clrtoeol();
                            addstr("Enter the RGB values seperated by <,>:");
                            int cur_x_pos = 38;
                            while ((num = getch()) != '\n') {
                                if ('0' <= num && num <= '9') {
                                    switch (cur) {
                                        case 0:
                                            cur_x_pos += 1;
                                            r *= 10;
                                            r += num - '0';
                                            addch(num);
                                            break;
                                        case 1:
                                            cur_x_pos += 1;
                                            g *= 10;
                                            g += num - '0';
                                            addch(num);
                                            break;
                                        case 2:
                                            b *= 10;
                                            b += num - '0';
                                            addch(num);
                                            break;
                                    }
                                } else if (num == ',' && cur < 2) {
                                    cur_x_pos += 1;
                                    cur += 1;
                                    addch(num);
                                } else if (num == 127 && cur_x_pos != 38) {
                                    switch (cur) {
                                        case 0:
                                            if (r != 0) {
                                                r /= 10;
                                                cur_x_pos -= 1;
                                            }
                                            break;
                                        case 1:
                                            if (g == 0) {
                                                cur -= 1;
                                            } else {
                                                g /= 10;
                                                cur_x_pos -= 1;
                                            }
                                            break;
                                        case 2:
                                            if (b == 0) {
                                                cur -= 1;
                                            } else {
                                                b /= 10;
                                                cur_x_pos -= 1;
                                            }
                                            break;
                                    }

                                    mvaddch(LINES - 1, cur_x_pos, ' ');
                                    move(LINES - 1, cur_x_pos);
                                }
                            }
                            move(LINES - 1, 0);
                            clrtoeol();
                            if (cur != 2) {

                            } else {
                                if (r > 255) {
                                    r = 255;
                                }
                                if (g > 255) {
                                    g = 255;
                                }
                                if (b > 255) {
                                    b = 255;
                                }
                                float colour_brightness =
                                    (r * 0.299 + 0.587 * g + 0.144 * b) / 255;
                                init_extended_color(16, r * 1000 / 255, g * 1000 / 255, b * 1000 / 255);
                                if (colour_brightness < 0.5) {
                                    init_extended_pair(16, COLOR_WHITE, 16);
                                } else {
                                    init_extended_pair(16, COLOR_BLACK, 16);
                                }
                                attron(COLOR_PAIR(16));
                                mvprintw(LINES - 1, 0, "Accept this color (y/n)");
                                attroff(COLOR_PAIR(16));
                                char choice;
                                while ((choice = getch()) != 'y' && choice != 'n');
                                if (choice == 'y') {
                                    accept = 1;
                                    user_colours[cmd_selected_color].r = r;
                                    user_colours[cmd_selected_color].g = g;
                                    user_colours[cmd_selected_color].b = b;
                                    user_colours[cmd_selected_color].a = 255;
                                }
                            }
                        }
                    } else {
                        move(LINES - 1, 0);
                        clrtoeol();
                        printw("Invalid input the command you gave in was %d%s",
                               cmd_selected_color, colour_command);
                        getch();
                    }
                }
                free(colour_command);
            } else if (compare_at(command, "cr",
                                  2)) { // this will be used to crop from the file
                // DONT FORGET TO WRITE IT
                int total = 0;
                int negative = 1;
                for (int pos = 2; pos < command_len; pos++) {
                    if (command[pos] == '-') {
                        negative = -1;
                    } else {
                        total = total * 10;
                        total += command[pos] - '0';
                    }
                }
                total *= negative;

            } else if (compare_at(command, "pr", 2)) { // CREATES THE PIXEL RATIO
                int total = 0;
                for (int pos = 2; pos < command_len; pos++) {
                    total = total * 10;
                    total += command[pos] - '0';
                }
                if (total != 0) {
                    if ((*height % total) == 0 && (*width % total) == 0) {
                        pixel_ratio = total;
                        printw("Pixel ratio =%d", pixel_ratio);
                        generate_colors_from_pixel_ratio(&*y, &*x, *height, *width);
                        load_colours(*width, *height);
                    } else {
                        mvaddstr(LINES - 1, 0, "Invalid input (press any key to continue)");
                        getch();
                    }
                }
            }
            move(LINES - 1, 0);
            clrtoeol();
            free(command);
            break;
    }
    loop_count = 1;
    loop_input_assigned = 0;
}
void generate_colors_from_pixel_ratio(unsigned int *y, unsigned int *x,
                                      int height, int width) {
    if (*y + y_buffer > height) {
        y_buffer = height - *y;
        if (y_buffer < 0) {
            y_buffer = 0;
            *y = height;
        }
    }
    if (*x + x_buffer > width) {
        x_buffer = width - *x;
        if (x_buffer < 0) {
            x_buffer = 0;
            *x = width;
        }
    }
    int y_pos = 0;
    int x_pos = 0;
    for (int i = 0; i < current_colours; i++) {
        free(colours[i].point);
    }

    free(colours);
    colours = malloc(sizeof(RGB_spectrum) * current_max_colours);
    colour_index = START_COLOR_INDEX;
    current_colours = 0;
    for (int y = 0; y < height; y += pixel_ratio) {
        x_pos = 0;
        for (int x = 0; x < width; x += pixel_ratio) {
            int r_total = 0;
            int g_total = 0;
            int b_total = 0;
            int a_total = 0;
            for (int rel_y = y; rel_y - y < pixel_ratio; rel_y++) {
                for (int rel_x = x; rel_x - x < pixel_ratio; rel_x++) {
                    r_total += total_image[rel_y][rel_x].r;
                    g_total += total_image[rel_y][rel_x].g;
                    b_total += total_image[rel_y][rel_x].b;
                    a_total += total_image[rel_y][rel_x].a;
                }
            }

            if (a_total != 0) {
                float p2 = pixel_ratio * pixel_ratio;
                RGB_spectrum colour;
                colour.r = r_total / p2;
                colour.g = g_total / p2;
                colour.b = b_total / p2;
                colour.length = 0;
                int colour_position = colours_contains(colour);
                if (colour_position != -1) {
                    colours[colour_position].point[colours[colour_position].length].x =
                        x_pos;
                    colours[colour_position].point[colours[colour_position].length].y =
                        y_pos;
                    colours[colour_position].length += 1;
                } else {
                    colour.point = malloc(sizeof( xy_point *) * 100000000);
                    colour.point[0].x = x_pos;
                    colour.point[0].y = y_pos;
                    colour.length += 1;
                    colours[current_colours] = colour;
                    colours[current_colours].code = colour_index;
                    current_colours += 1;
                }
            } else {
            }
            x_pos += 1;
        }
        y_pos += 1;
    }
}
void select_from_image(unsigned int *x, unsigned int *y, int width,
                       int height) {
    xy_point *coord = malloc(sizeof(xy_point) * 2000);
    coord[0].x = *x + x_buffer;
    coord[0].y = *y + y_buffer;
    int coord_count = 1;
    load_colours_select(coord, coord_count, width, height);
    char input;
    int run = 1;
    while (run) {
        move(*y, *x);
        input = getch();
        if ('0' <= input && input <= '9') {
            if (!loop_input_assigned) {
                loop_count = input - '0';

            } else {
                loop_count = loop_count * 10;
                loop_count += input - '0';
            }
        }
        switch (input) {
            case 27:
                if (loop_input_assigned) {
                    loop_count = 1;
                    loop_input_assigned = 0;
                } else {
                    run = 0;
                }
                break;
            case 'h':
                for (int i = 0; i < loop_count; i++) {
                    if (*x > 0) {
                        *x -= 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    } else if (x_buffer > 0) {
                        x_buffer -= 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(coord, coord_count, width, height);
                loop_count = 1;
                loop_input_assigned = 0;
                break;
            case 'l':
                for (int i = 0; i < loop_count && i < height / pixel_ratio; i++) {
                    if (*x + 1 < width / pixel_ratio && *x < COLS - 2) {
                        *x += 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    } else if (x_buffer + *x + 1 < width / pixel_ratio && *x == COLS - 2) {
                        x_buffer += 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(coord, coord_count, width, height);
                loop_count = 1;
                loop_input_assigned = 0;
                break;
            case 'j':
                for (int i = 0; i < loop_count && i < height / pixel_ratio; i++) {
                    if (*y + 1 < height / pixel_ratio && *y < LINES - 3) {
                        *y += 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    } else if (y_buffer + *y + 1 < height / pixel_ratio &&
                        *y == LINES - 3) {
                        y_buffer += 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(coord, coord_count, width, height);
                loop_count = 1;
                loop_input_assigned = 0;
                break;
            case 'k':
                for (int i = 0; i < loop_count; i++) {
                    if (*y > 0) {
                        *y -= 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    } else if (y_buffer > 0) {
                        y_buffer -= 1;
                        coord[coord_count].x = *x + x_buffer;
                        coord[coord_count].y = *y + y_buffer;
                        coord_count += 1;
                    }
                }
                load_colours_select(coord, coord_count, width, height);
                loop_count = 1;
                loop_input_assigned = 0;
                break;
            case 'c':
                for (int i = 0; i < coord_count; i++) {
                    cut_at(coord[i].x - x_buffer, coord[i].y - y_buffer, height, width);
                }
                loop_count = 1;
                loop_input_assigned = 0;
                run = 0;
                break;
            case 'd':
                for (int i = 0; i < coord_count && current_colour != -1; i++) {
                    int x = coord[i].x;
                    int y = coord[i].y;
                    draw_at(x - x_buffer, y - y_buffer, height, width,
                            user_colours[current_colour].r, user_colours[current_colour].g,
                            user_colours[current_colour].b);
                }
                loop_input_assigned = 0;
                loop_count = 0;
                run = 0;
                break;
        }
    }
    free(coord);
}

void load_colours_select( xy_point *points, int point_num, int width,
                         int height) {

    move(0, 0);
    attron(A_NORMAL);
    for (int y = 0; y < LINES - 2; y++) {
        for (int x = 0; x < COLS - 1; x++) {
            mvaddch(y, x, 'X');
        }
    }
    attroff(A_NORMAL);
    for (int i = 0; i < current_colours; i++) {
        colours[i].code = colour_index;
        init_extended_color(colour_index, colours[i].r * 1000 / 255,
                            colours[i].g * 1000 / 255, colours[i].b * 1000 / 255);
        init_extended_pair(colour_index, COLOR_BLACK, colour_index);
        attron(COLOR_PAIR(colour_index));
        for (int n = 0; n < colours[i].length; n++) {
            int cur_y = colours[i].point[n].y;
            int cur_x = colours[i].point[n].x;
            if (cur_x - x_buffer < COLS - 1 && cur_y - y_buffer < LINES - 2) {
                if (xy_point_in(points, point_num, cur_x, cur_y)) {
                    mvaddch(cur_y - y_buffer, cur_x - x_buffer, '0');

                } else {
                    mvaddch(cur_y - y_buffer, cur_x - x_buffer, ' ');
                }
            }
        }
        attroff(COLOR_PAIR(colour_index));
        colour_index += 1;
    }
    colour_index = START_COLOR_INDEX;
    refresh();
}
void select_fill_from_image(unsigned int *x, unsigned int *y, int width,
                            int height) {
    int init_y = *y + y_buffer;
    int init_x = *x + x_buffer;

    int range_y = 0;
    int range_x = 0;
    int run = 1;
    char input;

    load_colours_select_fill(init_y, init_x, range_y, range_x, width, height);

    while (run) {
        move(*y, *x);
        input = getch();
        if (input >= '0' && input <= '9') {
            if (!loop_input_assigned) {
                loop_count = input - '0';
                loop_input_assigned = 1;
            } else {
                loop_count *= 10;
                loop_count += input - '0';
            }
        }
        load_image_information(*x, *y, height, width);
        switch (input) {
            case 27:
                if (loop_input_assigned) {
                    loop_input_assigned = 0;
                    loop_count = 1;
                } else {
                    run = 0;
                }
                break;
            case 'h':
                for (int i = 0; i < loop_count; i++) {
                    if (*x != 0) {
                        *x -= 1;
                        range_x -= 1;
                    } else if (x_buffer > 0) {
                        x_buffer -= 1;
                        range_x -= 1;
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
                load_colours_select_fill(init_y, init_x, range_y, range_x, width, height);
                break;
            case 'j':
                for (int i = 0; i < loop_count && i < height; i++) {
                    if (*y + 1 < height / pixel_ratio && *y < LINES - 3) {
                        *y += 1;
                        range_y += 1;
                    } else if (y_buffer + *y + 1 < height / pixel_ratio &&
                        *y == LINES - 3) {
                        y_buffer += 1;
                        range_y += 1;
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
                load_colours_select_fill(init_y, init_x, range_y, range_x, width, height);
                break;
            case 'k':
                for (int i = 0; i < loop_count; i++) {
                    if (*y != 0) {
                        *y -= 1;
                        range_y -= 1;
                    } else if (y_buffer > 0) {
                        y_buffer -= 1;
                        range_y -= 1;
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
                load_colours_select_fill(init_y, init_x, range_y, range_x, width, height);
                break;
            case 'l':
                for (int i = 0; i < loop_count && i < width; i++) {
                    if (*x + 1 < width / pixel_ratio && *x < COLS - 2) {
                        *x += 1;
                        range_x += 1;
                    } else if (x_buffer + *x + 1 < width / pixel_ratio && *x == COLS - 2) {
                        x_buffer += 1;
                        range_x += 1;
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
                load_colours_select_fill(init_y, init_x, range_y, range_x, width, height);
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
                        cut_at(x_pos - x_buffer, y_pos - y_buffer, width, height);
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
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
                for (int y_pos = tmp_init_y1; y_pos <= y_range1 && current_colour != -1;
                y_pos++) {
                    for (int x_pos = tmp_init_x1; x_pos <= x_range1; x_pos++) {
                        draw_at(x_pos - x_buffer, y_pos - y_buffer, width, height,
                                user_colours[current_colour].r,
                                user_colours[current_colour].g,
                                user_colours[current_colour].b);
                    }
                }
                loop_input_assigned = 0;
                loop_count = 1;
                run = 0;

                break;
        }
    }
}

void load_colours_select_fill(int init_y, int init_x, int range_y, int range_x,
                              int width, int height) {
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
    for (int i = 0; i < current_colours; i++) {
        colours[i].code = colour_index;
        init_extended_color(colour_index, colours[i].r * 1000 / 255,
                   colours[i].g * 1000 / 255, colours[i].b * 1000 / 255);
        init_extended_pair(colour_index, COLOR_BLACK, colour_index);
        attron(COLOR_PAIR(colour_index));
        for (int n = 0; n < colours[i].length; n++) {
            int cur_y = colours[i].point[n].y;
            int cur_x = colours[i].point[n].x;
            if ((cur_y >= y_buffer && cur_y < LINES - 2 + y_buffer) &&
                (cur_x >= x_buffer && cur_x < COLS - 1 + x_buffer) &&
                cur_x - x_buffer < COLS - 1 && cur_y - y_buffer < LINES - 2) {
                if (in_range(init_y, y_range, cur_y) &&
                    in_range(init_x, x_range, cur_x)) {
                    mvaddch(cur_y - y_buffer, cur_x - x_buffer, '0');
                } else {
                    mvaddch(cur_y - y_buffer, cur_x - x_buffer, ' ');
                }
            }
        }
        attroff(COLOR_PAIR(colour_index));
        colour_index += 1;
    }
    colour_index = START_COLOR_INDEX;
    refresh();
}
int load_colours(int width, int height) {
    move(0, 0);
    attron(A_NORMAL);
    for (int y = 0; y < LINES - 2; y++) {
        for (int x = 0; x < COLS - 1; x++) {
            mvaddch(y, x, 'X');
        }
    }
    attroff(A_NORMAL);
    for (int i = 0; i < current_colours; i++) {
        colours[i].code = colour_index;
        init_extended_color(colour_index, colours[i].r * 1000 / 255,
                            colours[i].g * 1000 / 255, colours[i].b * 1000 / 255);
        init_extended_pair(colour_index, COLOR_BLACK, colour_index);
        attron(COLOR_PAIR(colour_index));
        for (int n = 0; n < colours[i].length; n++) {
            int cur_y = colours[i].point[n].y;
            int cur_x = colours[i].point[n].x;
            if ((cur_y >= y_buffer && cur_y - y_buffer < LINES - 2) &&
                (cur_x >= x_buffer && cur_x - x_buffer < COLS - 1))
                mvaddch(cur_y - y_buffer, cur_x - x_buffer, ' ');
        }
        attroff(COLOR_PAIR(colour_index));
        colour_index += 1;
    }
    colour_index = START_COLOR_INDEX;
    refresh();
    return 0;
}
void load_image_information(int x, int y, int height, int width) {
    move(LINES - 2, 0);
    clrtoeol();
    mvprintw(LINES - 2, 10, "%s             %d:%d", filename, height, width);
    mvprintw(LINES - 2, COLS - 20, "(%d,%d)     %d", y + y_buffer, x + x_buffer,
             loop_count);
}
void create_window(int height, int width) {
    win = newwin(width, height, 0, 0);
    initscr();
    keypad(win, true);
    start_color();
    noecho();
    cbreak();
    curs_set(0);
}
