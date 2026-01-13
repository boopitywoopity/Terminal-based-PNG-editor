#include "util.h"
#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#define BACKSPACE 127

// this will read the command from the user
char *get_user_command(int *len);
void process_user_command(image_container *img, program_info *info, unsigned int *x, unsigned int *y, char *command, int len, int *quit);

void key_press(image_container *img, program_info *info, unsigned int *x, unsigned int *y, char input, int *quit) {

    // this will assign the loop count
    if (input >= '0' && input <= '9') {
        // if there's no loop count then assign it
        if (!info->loop_count_assigned) {
            info->loop_count = input - '0';
            info->loop_count_assigned = true;
        }
        // otherwise just increase it
        else {
            info->loop_count *= 10;
            info->loop_count += input - '0';
        }
        return;
    }
    switch (input) {
        case 'h':
            for (int i = 0; i < info->loop_count; i++) {
                if (*x != 0) {
                    *x -= 1;
                }
                else if (img->x_buffer > 0) {
                    img->x_buffer -= 1;
                }
            }
            load_colours(img);
            break;
        case 'j':
            for (int i = 0; i < info->loop_count && i < img->height / img->pixel_ratio; i++) {
                if (*y + 1 < img->height / img->pixel_ratio && *y < LINES - 3) {
                    *y += 1;
                }
                else if (img->y_buffer + *y + 1 < img->height / img->pixel_ratio && *y == LINES - 3) {
                    img->y_buffer += 1;
                }
            }
            load_colours(img);
            break;
        case 'k':
            for (int i = 0; i < info->loop_count; i++) {
                if (*y != 0) {
                    *y -= 1;
                }
                else if (img->y_buffer > 0) {
                    img->y_buffer -= 1;
                }
            }
            load_colours(img);
            break;
        case 'l':
            for (int i = 0; i < info->loop_count && i < img->width / img->pixel_ratio; i++) {
                if (*x + 1 < img->width / img->pixel_ratio && *x < COLS - 2) {
                    *x += 1;
                }
                else if (img->x_buffer + *x + 1 < img->width / img->pixel_ratio && *x == COLS - 2) {
                    img->x_buffer += 1;
                }
            }
            load_colours(img);
            break;
        // case 'd':
        //     if (info->current_colour == -1)
        //         break;
        //     draw_at(img, *x, *y, info->user_colours[info->current_colour].r, info->user_colours[info->current_colour].g, info->user_colours[info->current_colour].b);
        //     generate_colours_from_pixel_ratio(img, y, x);
        //     load_colours(img);
        //     break;
        // case 'c':
        //     cut_at(img, *x, *y);
        //     generate_colours_from_pixel_ratio(img, y, x);
        //     load_colours(img);
        //     break;
        // case 'v':
        //     select_fill_from_image(img, info, x, y);
        //     generate_colours_from_pixel_ratio(img, y, x);
        //     load_colours(img);
        //     break;
        // case 'V':
        //     select_from_image(img, info, x, y);
        //     generate_colours_from_pixel_ratio(img, y, x);
        //     load_colours(img);
        //     break;
        case ':': // handles commands from the user
            // TODO: REMOVE WHEN ITS FINISHED
            if (true)
                ; // this is there because if it isnt the lsp screams at me that there is
            int len;
            char *command = get_user_command(&len);
            process_user_command(img, info, x, y, command, len, quit);

            move(LINES - 1, 0);
            clrtoeol();
            free(command);
            break;
    }
    info->loop_count = 1;
    info->loop_count_assigned = false;
}


char *get_user_command(int *len){
    char *command = malloc(sizeof(char) * 100);
    *len = 0;
    char input;
    int xpos = 1;
    int value = 0;
    mvaddch(LINES - 1, 0, ':');

    // get the full command from the user, then compare it to see if it matches with the existing ones
    while ((input = getch()) != '\n') { // get the command input from the user
        if (input == BACKSPACE && xpos > 1) {
            command[(*(len))--] = '\0';
            mvaddch(LINES - 1, --xpos, ' ');
            move(LINES - 1, xpos);
        }
        else if (input != BACKSPACE) {
            value = value * 10 + input - '0';
            command[(*(len))++] = input;
            mvaddch(LINES - 1, xpos++, input);
        }
        else if (input == 27){
            *len = 0;
            return NULL;
        }
    }

    return command;
}

void user_colour(program_info *info);

void process_user_command(image_container *img, program_info *info, unsigned int *x, unsigned int *y, char *command, int len, int *quit){
    if (compare_at(command, "q", 1)) {
        *quit = 1;
    }
    else if (compare_at(command, "col", 3)) {
        // user_colour(info);
    }
    else if (compare_at(command, "cr", 2)) { // this will be used to crop from the file
        int total = 0;
        int negative = 1;
        for (int pos = 2; pos < len; pos++) {
            if (command[pos] == '-') {
                negative = -1;
            }
            else {
                total = total * 10;
                total += command[pos] - '0';
            }
        }
        total *= negative;

        // TODO: implement cropping
    }
    // else if (compare_at(command, "pr", 2)) { // create the pixel ratio
    //     int total = 0;
    //     for (int pos = 2; pos < len; pos++) {
    //         total = total * 10;
    //         total += command[pos] - '0';
    //     }
    //     if (total != 0) {
    //         if ((img->height % total) == 0 && (img->width % total) == 0) {
    //             img->pixel_ratio = total;
    //             printw("Pixel ratio =%d", img->pixel_ratio);
    //             generate_colours_from_pixel_ratio(img, y, x);
    //             load_colours(img);
    //         }
    //         else {
    //             mvaddstr(LINES - 1, 0, "Invalid input (press any key to continue)");
    //             getch();
    //         }
    //     }
    // }
}

void display_defined_colours(program_info *info);
bool read_colour_command(int *cmd_selected_colour, char **cmd, int *cmd_len);

// void user_colour(program_info *info){
//     // display_defined_colours(info);
//
//     int col_cmd_len;
//     char *colour_command;
//     // this is the selected colour that the user has chosen for the action
//     int cmd_selected_colour = -1;
//     if (!read_colour_command(&cmd_selected_colour, &colour_command, &col_cmd_len)){
//         fprintf(stderr, "Error: failed to read colour command\n");
//         return;
//     }
//
//     if (cmd_selected_colour == -1) {
//         move(LINES - 1, 0);
//         clrtoeol();
//         printw("Invalid input the command you gave in was %d%s",
//                cmd_selected_colour, colour_command);
//         getch();
//         return;
//     }
//
//     if (colour_command[col_cmd_len] != '\0') {
//         colour_command[col_cmd_len] = '\0';
//     }
//     if (strcmp(colour_command, "-use") == 0) { // assign this colour to the currently used user colour
//         info->current_colour = cmd_selected_colour;
//     }
//     else if (strcmp(colour_command, "-set") == 0) { // assign the selected colour to a value defined by the user
//         int accept = 0;
//         while (!accept) {
//             int r = 0;
//             int g = 0;
//             int b = 0;
//
//             int cur = 0;
//             char num;
//             move(LINES - 1, 0);
//             clrtoeol();
//             addstr("Enter the RGB values seperated by <,>:");
//             int cur_x_pos = 38;
//             while ((num = getch()) != '\n') {
//                 if ('0' <= num && num <= '9') {
//                     switch (cur) {
//                         case 0:
//                             cur_x_pos += 1;
//                             r *= 10;
//                             r += num - '0';
//                             addch(num);
//                             break;
//                         case 1:
//                             cur_x_pos += 1;
//                             g *= 10;
//                             g += num - '0';
//                             addch(num);
//                             break;
//                         case 2:
//                             b *= 10;
//                             b += num - '0';
//                             addch(num);
//                             break;
//                     }
//                 }
//                 else if (num == ',' && cur < 2) {
//                     cur_x_pos += 1;
//                     cur += 1;
//                     addch(num);
//                 }
//                 else if (num == BACKSPACE && cur_x_pos != 38) {
//                     switch (cur) {
//                         case 0:
//                             if (r != 0) {
//                                 r /= 10;
//                                 cur_x_pos -= 1;
//                             }
//                             break;
//                         case 1:
//                             if (g == 0) {
//                                 cur -= 1;
//                             }
//                             else {
//                                 g /= 10;
//                                 cur_x_pos -= 1;
//                             }
//                             break;
//                         case 2:
//                             if (b == 0) {
//                                 cur -= 1;
//                             }
//                             else {
//                                 b /= 10;
//                                 cur_x_pos -= 1;
//                             }
//                             break;
//                     }
//
//                     mvaddch(LINES - 1, cur_x_pos, ' ');
//                     move(LINES - 1, cur_x_pos);
//                 }
//             }
//             move(LINES - 1, 0);
//             clrtoeol();
//             // TODO: finish this (this is the user colour definition)
//             if (cur != 2) {
//
//             }
//             else {
//                 if (r > 255) {
//                     r = 255;
//                 }
//                 if (g > 255) {
//                     g = 255;
//                 }
//                 if (b > 255) {
//                     b = 255;
//                 }
//                 float colour_brightness =
//                     (r * 0.299 + 0.587 * g + 0.144 * b) / 255;
//                 init_extended_color(16, r * 1000 / 255, g * 1000 / 255, b * 1000 / 255);
//                 if (colour_brightness < 0.5) {
//                     init_extended_pair(16, COLOR_WHITE, 16);
//                 }
//                 else {
//                     init_extended_pair(16, COLOR_BLACK, 16);
//                 }
//                 attron(COLOR_PAIR(16));
//                 mvprintw(LINES - 1, 0, "Accept this colour (y/n)");
//                 attroff(COLOR_PAIR(16));
//                 char choice;
//                 while ((choice = getch()) != 'y' && choice != 'n');
//                 if (choice == 'y') {
//                     accept = 1;
//                     info->user_colours[cmd_selected_colour].r = r;
//                     info->user_colours[cmd_selected_colour].g = g;
//                     info->user_colours[cmd_selected_colour].b = b;
//                     info->user_colours[cmd_selected_colour].a = 255;
//                 }
//             }
//         }
//     }
//     free(colour_command);
// }
//
// void dislpay_defined_colours(program_info *info){
//     for (int i = LINES - 3; i < LINES - 1; i++) {
//         move(i, 0);
//         clrtoeol();
//     }
//     for (int i = 0; i < 10; i++) { // generates the colours for the user to preview the colour templates
//         float colour_brightness =
//             (info->user_colours[i].r * 0.299 + 0.587 * info->user_colours[i].g +
//             0.144 * info->user_colours[i].b) /
//             255;
//
//         init_extended_color(17 + i, info->user_colours[i].r * 1000 / 255,
//                             info->user_colours[i].g * 1000 / 255,
//                             info->user_colours[i].b * 1000 / 255);
//         if (colour_brightness < 0.5) {
//             init_extended_pair(17 + i, COLOR_WHITE, 17 + i);
//         }
//         else {
//             init_extended_pair(17 + i, COLOR_BLACK, 17 + i);
//         }
//         attron(COLOR_PAIR(17 + i));
//         mvprintw(LINES - 3, i * 10, "Colour %d", i);
//         attroff(COLOR_PAIR(17 + i));
//     }
//     move(LINES - 1, 0);
//     clrtoeol();
// }

// bool read_colour_command(int *cmd_selected_colour, char **cmd, int *cmd_len){
//     int len;
//     char *unformatted_cmd = get_user_command(&len);
//     if (unformatted_cmd == NULL){
//         return false;
//     }
//     else if (unformatted_cmd[0] >= '0' && unformatted_cmd[0] <= '9'){
//         *cmd_selected_colour = unformatted_cmd[0] - '0';
//     }
//     else { // the user has not given in a valid command, return false
//         return false;
//     }
//
//     // trim the whitespace
//     // I can't think of what to name this variable, will get changed later
//     char *cmd_pntr = unformatted_cmd+1;
//     while (*cmd_pntr == ' ' && *cmd_pntr != '\0' && len > 0){
//         cmd_pntr += 1;
//         len -= 1;
//     }
//     *cmd = malloc(sizeof(char)*100);
//     if (*cmd == NULL){
//         exit(EXIT_FAILURE);
//     }
//     strcpy(*cmd, cmd_pntr);
//     *cmd_len = len;
//
//     free(unformatted_cmd);
//     return true;
// }


