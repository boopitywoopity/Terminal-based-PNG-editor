#!/bin/zsh
gcc main.c avl.c util.c image-loader.c user-colours.c keyboard-input.c terminal-drawer.c -o run -lm -lncursesw
