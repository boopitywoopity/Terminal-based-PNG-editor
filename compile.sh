#!/bin/zsh
gcc main.c util.c image-loader.c user-colours.c keyboard-input.c -o run -lm -lncursesw
