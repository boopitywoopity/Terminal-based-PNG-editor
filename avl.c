#include "util.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void cleanup_rec(tree *n);

void cleanup_tree(tree **root){
    cleanup_rec(*root);
}

void cleanup_rec(tree *n){
    if (n->l != NULL){
        cleanup_rec(n->l);
    }
    if (n->r != NULL){
        cleanup_rec(n->r);
    }
    free(n);
}

bool contains(tree **root, const uint32_t target){
    tree *current = *root;

    while (current != NULL){
        if (current->colour_32 == target){ // this is the colour, return true
            return true;
        }
        else { // colour not found, keep going
            if (target < current->colour_32){
                if (current->l != NULL){
                    current = current->l;
                }
                else {
                    return false;
                }
            }
            else if (target > current->colour_32){
                if (current->r != NULL){
                    current = current->r;
                }
                else {
                    return false;
                }
            }
        }
    }
    return false;
}

// returns 0 if not found
uint32_t get_colour_code(tree **root, const uint32_t target){
    tree *t = *root;
    tree *current = t;

    while (current != NULL){
        if (current->colour_32 == target){ // this is the colour, return the colour code
            return current->colour_code;
        }
        else { // colour not found, keep going
            if (target < current->colour_32){
                if (current->l != NULL){
                    current = current->l;
                }
                else {
                    return 0;
                }
            }
            else if (target > current->colour_32){
                if (current->r != NULL){
                    current = current->r;
                }
                else {
                    return 0;
                }
            }
        }
    }
    return 0;
}

tree *insert_node(tree *node, tree *newnode, uint32_t key);
int initialize_colour(tree *newnode, const uint8_t *rgb, uint32_t next_colour_code);
void update_height(tree *n);
int get_bf(tree *l, tree *r);
tree *r_r(tree *y);
tree *l_r(tree *y);
int height(tree *n);

void insert(tree **root, const uint32_t target){
    tree *newnode = malloc(sizeof(tree));
    *newnode = (tree){
        .l = NULL,
        .r = NULL,
        .height = 1,
        .colour_32 = target,
        .colour_code = 0
    };

    newnode->colour[0] = (target >> 24) & 0xFF; // red
    newnode->colour[1] = (target >> 16) & 0xFF; // green
    newnode->colour[2] = (target >> 8)  & 0xFF; // blue
    if ((target << 24) == 0){ // the image is transparent there's nothing there
        free(newnode);
        return;
    }

    tree *t = *root;
    static uint32_t next_colour_code = START_COLOR_INDEX;

    if (*root == NULL) {
        *root = newnode;
        next_colour_code += initialize_colour(newnode, newnode->colour, next_colour_code);
    }
    else {
        *root = insert_node(*root, newnode, target);
        next_colour_code += initialize_colour(newnode, newnode->colour, next_colour_code);
    }
}
tree *insert_node(tree *node, tree *newnode, uint32_t key){
    if (node == NULL){
        return newnode;
    }

    if (key < node->colour_32){
        node->l = insert_node(node->l, newnode, key);
    }
    else if (key > node->colour_32){
        node->r = insert_node(node->r, newnode, key);
    }
    else {
        return node;
    }

    update_height(node);
    int bf = get_bf(node->l, node->r);
    if (bf > 1 && node->l != NULL && key < node->l->colour_32){ // LL
        return r_r(node);
    }
    if (bf > 1 && node->l != NULL && key > node->l->colour_32){ // LR
        node->l = l_r(node->l);
        return r_r(node);
    }
    if (bf < -1 && node->r != NULL && key > node->r->colour_32){ // RR
        return l_r(node);
    }
    if (bf < -1 && node->r != NULL && key < node->r->colour_32){ // RL
        node->r = r_r(node->r);
        return l_r(node);
    }

    return node;
}

int initialize_colour(tree *newnode, const uint8_t *rgb, uint32_t next_colour_code){
    // initialize the colour
    fprintf(stderr, "Initializing colour rgb(%d,%d,%d)\n", rgb[0], rgb[1], rgb[2]);
    init_extended_color(next_colour_code, rgb[0]*1000/255, rgb[1]*1000/255, rgb[2]*1000/255);
    // init_extended_color(next_colour_code, rgb[0], rgb[1], rgb[2]);
    init_extended_pair(next_colour_code, COLOR_BLACK, next_colour_code);
    newnode->colour_code += next_colour_code;
    return 1;
}

int get_bf(tree *l, tree *r){
    int lval = 0;
    int rval = 0;

    if (l != NULL){
        lval = l->height;
    }
    if (r != NULL){
        rval = r->height;
    }

    return lval - rval;
}

int height(tree *n) {
    return n ? n->height : 0;
}

void update_height(tree *n) {
    n->height = 1 + (height(n->l) > height(n->r)
                    ? height(n->l)
                    : height(n->r));
}

tree *r_r(tree *y){
    if (!y || !y->l)
        return y;
    tree *x = y->l;
    tree *T2 = x->r;

    x->r = y;
    y->l = T2;

    update_height(y);
    update_height(x);

    return x;
}

tree *l_r(tree *y){
    if (!y || !y->r)
        return y;
    tree *x = y->r;
    tree *T2 = x->l;

    x->l = y;
    y->r = T2;

    update_height(y);
    update_height(x);

    return x;
}
