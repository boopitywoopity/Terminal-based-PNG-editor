#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/stb_image.h"
#include "util.h"

static image_container generate_default(const int width, const int height){
    image_container img = {
        .x_buffer = 1,
        .y_buffer = 1,
        .current_max_colours = 1000,
        .width = width,
        .height = height,
        .pixel_ratio = 1,
        .colour_index = 0,
        .current_colours = 0,
    };

    img.root = NULL;
    // img.colours = malloc(sizeof(RGB_spectrum) * img.current_max_colours);

    img.total_image = malloc(sizeof(point_w_colour *) * (height+1));

    if (img.total_image == NULL) { // there was a failure to allocate enough memory for the total image
        endwin();
        printf("There was an issue allocating memory for the total image\n");
        exit(EXIT_FAILURE);
    }

    return img;
}

image_container load_image(unsigned char *image, unsigned int width, unsigned int height){
    image_container img = generate_default(width, height);

    for (int y = 0; y < height; y++) {
        img.total_image[y] = malloc(sizeof(point_w_colour) * (width+1)); // alocates the memory per line

        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            uint32_t value = *(uint32_t *)&image[index];
            img.total_image[y][x] = value;
            if (!contains(&img.root, value)){
                insert(&img.root, value);
                img.current_colours += 1;
            }
        }
    }
    return img;
}

// image_container load_image(unsigned char *image, unsigned int width, unsigned int height){
//     image_container img = generate_default(width, height);
//
//     for (int y = 0; y < height; y++) {
//         img.total_image[y] = malloc(sizeof(point_w_colour) * (width+1)); // alocates the memory per line
//         // total_image[y] = malloc(sizeof( point_w_colour *) * 400000); // alocates the memory per line
//         for (int x = 0; x < width; x++) {
//             int index = (y * width + x) * 4;
//             img.total_image[y][x] = *(uint32_t *)&image[index];
//
//             // this makes sure the alpha (transparency) is not set to 0 which
//             // would make the pixel invisible
//             if (image[index + 3] != 0) {
//                 // create a new colour, if the colour already exists just ignore this and use the existing one, otherwise use it
//                 // a better way to do this might be to hash the rgb values and then look to see if it exists
//
//                 // this colour already exists, bind the point to the colour
//                 int colour_position = colours_contains(&img, image[index], image[index+1], image[index+2]);
//                 if (colour_position != -1) {
//                     // create a new point and add it to an existing colour
//                     xy_point point;
//                     point.x = x;
//                     point.y = y;
//
//                     // adds the new point to the points of this colour
//                     if (img.colours[colour_position].max_length == img.colours[colour_position].length){
//                         img.colours[colour_position].max_length *= 2;
//                         img.colours[colour_position].point = realloc(img.colours[colour_position].point, sizeof(xy_point)*img.colours[colour_position].max_length);
//                     }
//                     img.colours[colour_position].point[img.colours[colour_position].length++] = point;
//                 }
//                 // this colour does not exist bind the new colour to the colours and then bind the current location to the colour
//                 else {
//                     RGB_spectrum colour;
//                     colour.r = image[index];
//                     colour.g = image[index + 1];
//                     colour.b = image[index + 2];
//                     colour.length = 0;
//                     if (img.current_colours+1 == INT_MAX){ // you have too many colours, kill the program
//                         endwin();
//                         stbi_image_free(image);
//                         // Free all the colours
//                         for (int i = 0; i < img.current_colours; i++) {
//                             free(img.colours[i].point);
//                         }
//
//                         free(img.colours);
//                         free(img.total_image);
//                         printf("The image you are attempting to load has esceeded the maximum ammount of colours available\n");
//
//                         exit(EXIT_FAILURE);
//                     }
//
//                     if (img.current_colours == img.current_max_colours) { // you have exceeded the maximum ammount of colours that is currently allocated, resize the array
//                         img.current_max_colours *= 2;
//                         img.colours = realloc(img.colours, sizeof(RGB_spectrum)*img.current_max_colours);
//                     }
//
//                     // create another point
//                     xy_point point;
//                     point.x = x;
//                     point.y = y;
//
//                     colour.max_length = MAX_STARTING_POINT_COUNT;
//                     colour.point = malloc(sizeof(xy_point) * colour.max_length);
//                     colour.point[colour.length++] = point;
//                     colour.max_length = MAX_STARTING_POINT_COUNT;
//                     img.colours[img.current_colours++] = colour;
//                 }
//             }
//         }
//     }
//     return img;
// }
