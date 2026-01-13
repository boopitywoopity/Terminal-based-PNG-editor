#include "util.h"
#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// load the entire image
int load_colours(image_container *img){
    move(0, 0); // move to the start of the screen to begin loading
    clrtobot();
    for (int x=img->x_buffer; x-img->x_buffer<COLS && x < img->width; x++){
        for (int y=img->y_buffer; y-img->y_buffer<LINES-2 && y < img->height;y++){
            uint8_t rgb[4];
            memcpy(rgb, &img->total_image[y][x], sizeof(uint32_t));
            if (!contains(&img->root, img->total_image[y][x])){ // the image is transparent there's nothing there
                attron(A_NORMAL);
                mvaddch(y-img->y_buffer, x-img->x_buffer, 'X');
                attroff(A_NORMAL);
            }
            else {
                uint32_t colour = get_colour_code(&img->root, img->total_image[y][x]);
                attron(COLOR_PAIR(colour));
                mvaddch(y-img->y_buffer, x-img->x_buffer, ' ');
                attroff(COLOR_PAIR(colour));
            }
        }
    }
    return 0;
}

void load_image_information(image_container *img, program_info *info, int x, int y) {
    move(LINES - 2, 0);
    clrtoeol();
    mvprintw(LINES - 2, 10, "%s             %d:%d", info->fname, img->height, img->width);
    mvprintw(LINES - 2, COLS - 20, "(%d,%d)     %d", y + img->y_buffer, x + img->x_buffer, info->loop_count);
}
