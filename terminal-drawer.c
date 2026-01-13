#include "util.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// load the entire image
int load_colours(image_container *img){
    move(0, 0); // move to the start of the screen to begin loading
    for (int x=0; x<COLS; x++){
        for (int y=0; y<LINES;y++){
            uint8_t rgb[4];
            memcpy(rgb, &img->total_image[y][x], sizeof(uint32_t));
            if (rgb[3] == 0){ // the image is transparent there's nothing there
                attron(A_NORMAL);
                mvaddch(y, x, 'X');
                attroff(A_NORMAL);
            }
            else {

            }

        }
    }
    return 0;
}

// // Load the entire image
// int load_colours(image_container *img) {
//     // TODO: rewrite this such that it's not fucking stupid
//     move(0, 0);
//     attron(A_NORMAL);
//     for (int y = 0; y < LINES - 2; y++) {
//         for (int x = 0; x < COLS - 1; x++) {
//             mvaddch(y, x, 'X');
//         }
//     }
//     attroff(A_NORMAL);
//     for (int i = 0; i < img->current_colours; i++) {
//         img->colours[i].code = img->colour_index;
//         init_extended_color(img->colour_index, img->colours[i].r * 1000 / 255, img->colours[i].g * 1000 / 255, img->colours[i].b * 1000 / 255);
//         init_extended_pair(img->colour_index, COLOR_BLACK, img->colour_index);
//         attron(COLOR_PAIR(img->colour_index));
//         for (int n = 0; n < img->colours[i].length; n++) {
//             int cur_y = img->colours[i].point[n].y;
//             int cur_x = img->colours[i].point[n].x;
//             if ((cur_y >= img->y_buffer && cur_y - img->y_buffer < LINES - 2)
//                 && (cur_x >= img->x_buffer && cur_x - img->x_buffer < COLS - 1)){
//                 mvaddch(cur_y - img->y_buffer, cur_x - img->x_buffer, ' ');
//             }
//         }
//         attroff(COLOR_PAIR(img->colour_index));
//         img->colour_index += 1;
//     }
//     img->colour_index = START_COLOR_INDEX;
//     refresh();
//     return 0;
// }

void load_image_information(image_container *img, program_info *info, int x, int y) {
    move(LINES - 2, 0);
    clrtoeol();
    mvprintw(LINES - 2, 10, "%s             %d:%d", info->fname, img->height, img->width);
    mvprintw(LINES - 2, COLS - 20, "(%d,%d)     %d", y + img->y_buffer, x + img->x_buffer, info->loop_count);
}
