#include "util.h"
#include <stdlib.h>



program_info generate_default_program_info(char *fname){
    program_info info = {
        .fname = fname,
        .current_colour = -1,
        .loop_count = 1,
        .loop_count_assigned = false,
    };

    // these are user defined colours for drawing
    // the user can only define 10 colours (for now might change later)
    info.user_colours = malloc(sizeof( point_w_colour) * 10);
    for (int user_clr_pos = 0; user_clr_pos < 10; user_clr_pos++) { // define them all as black by default
        info.user_colours[user_clr_pos].r = 0;
        info.user_colours[user_clr_pos].g = 0;
        info.user_colours[user_clr_pos].b = 0;
    }
    return info;
}

