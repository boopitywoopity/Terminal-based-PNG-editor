#!/bin/zsh
gcc main.c util.c image-loader.c globals.c -o run -lm -lncursesw
