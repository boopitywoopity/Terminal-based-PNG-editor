#!/bin/zsh
gcc main.c util.c image-loader.c user-colours.c -o run -lm -lncursesw
