#include "util.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



bool contains(tree **root, const uint32_t target){
    tree *t = *root;
    tree *current = t;

    while (current->l != NULL && current->r != NULL){
        uint32_t current_colour=(uint32_t)current->colour[0] << 16 |
                                (uint32_t)current->colour[1] << 8  |
                                (uint32_t)current->colour[2];
        if (current_colour == target){ // this is the colour, return true
            return true;
        }
        else { // colour not found, keep going
            if (target < current_colour){
                if (current->l != NULL){
                    current = current->l;
                }
                else {
                    return false;
                }
            }
            else if (target > current_colour){
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
uint64_t get_colour_code(tree **root, const uint32_t target){
    tree *t = *root;
    tree *current = t;

    while (current->l != NULL && current->r != NULL){
        uint32_t current_colour=(uint32_t)current->colour[0] << 16 |
                                (uint32_t)current->colour[1] << 8  |
                                (uint32_t)current->colour[2];
        if (current_colour == target){ // this is the colour, return the colour code
            return current->colour_code;
        }
        else { // colour not found, keep going
            if (target < current_colour){
                if (current->l != NULL){
                    current = current->l;
                }
                else {
                    return 0;
                }
            }
            else if (target > current_colour){
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

int insert_node(tree *root, tree **stack, tree *newnode, uint32_t target);
int initialize_colour(tree *newnode, const uint8_t *rgb, uint64_t next_colour_code);
// returns the index of the first imbalanced node that is unbalanced,
// the previous value on the stack will be that node's parent
int update_height(tree **stack, int stack_count);
void balance_tree(tree **root, tree **stack, int imbalanced_index);
int get_bf(tree *l, tree *r);
tree *r_r(tree *y);
tree *l_r(tree *y);

void insert(tree **root, const uint32_t target){
    tree *newnode = malloc(sizeof(tree));
    *newnode = (tree){
        .l = NULL,
        .r = NULL,
        .height = 1,
        .colour_code = 0
    };

    uint32_t value = target;
    uint8_t rgb[4];
    memcpy(rgb, &value, sizeof value);
    if (rgb[3] != 0){ // the image is transparent there's nothing there
        return;
    }

    newnode->colour[0] = rgb[0];
    newnode->colour[1] = rgb[1];
    newnode->colour[2] = rgb[2];

    tree *t = *root;
    static uint64_t next_colour_code = START_COLOR_INDEX;

    if (*root == NULL) {
        *root = newnode;
        next_colour_code += initialize_colour(newnode, rgb, next_colour_code);
    }
    else {
        tree **stack = malloc(sizeof(tree*)*128); // if the tree has more than 128 depth what the fuck are you doing
        int stack_count = insert_node(*root, stack, newnode, target);
        next_colour_code += initialize_colour(newnode, rgb, next_colour_code);
        int i = update_height(stack, stack_count);
        balance_tree(root, stack, i);
    }
}

int insert_node(tree *root, tree **stack, tree *newnode, uint32_t target){
    tree *current = root;
    int stack_count = 0;
    while (current->l != NULL && current->r != NULL){
        uint32_t current_colour = (uint32_t)current->colour[0] << 16 |
                                  (uint32_t)current->colour[1] << 8  |
                                  (uint32_t)current->colour[2];

        if (current != NULL && current_colour == target){ // this colour already exists, return
            free(stack);
            return 0;
        }
        else { // colour not found, keep going
            if (target < current_colour){
                if (current->l != NULL){
                    stack_count += 1;
                    stack[stack_count++] = current;

                    current = current->l;
                }
                else {
                    current->l = newnode;
                    break;
                }
            }
            else if (target > current_colour){
                if (current->r != NULL){
                    stack_count += 1;
                    stack[stack_count++] = current;

                    current = current->r;
                }
                else {
                    current->r = newnode;
                    break;
                }
            }
        }
    }

    return stack_count;
}

void balance_tree(tree **root, tree **stack, int imbalanced_index){
        if (imbalanced_index != -1) {
            tree *z = stack[imbalanced_index];
            tree *parent = (imbalanced_index > 0) ? stack[imbalanced_index - 1] : NULL;

            int bfz = get_bf(z->l, z->r);

            tree *new_subroot = NULL;

            if (bfz < -1 && get_bf(z->l->l, z->l->r) <= 0) { // LL
                new_subroot = r_r(z);
            }
            else if (bfz > 1 && get_bf(z->r->l, z->r->r) >= 0) { // RR
                new_subroot = l_r(z);
            }
            else if (bfz < -1 && get_bf(z->l->l, z->l->r) > 0) { // LR
                z->l = l_r(z->l);
                new_subroot = r_r(z);
            }
            else if (bfz > 1 && get_bf(z->r->l, z->r->r) < 0) { // RR
                z->r = r_r(z->r);
                new_subroot = l_r(z);
            }

            // Reattach to parent or root
            if (new_subroot != NULL) {
                if (parent == NULL){
                    *root = new_subroot;
                }
                else if (parent->l == z){
                    parent->l = new_subroot;
                }
                else {
                    parent->r = new_subroot;
                }
            }
        }
        free(stack);
}

int initialize_colour(tree *newnode, const uint8_t *rgb, uint64_t next_colour_code){
    // initialize the colour
    init_extended_color(next_colour_code, rgb[0], rgb[1], rgb[2]);
    init_extended_pair(next_colour_code, COLOR_BLACK, next_colour_code);
    newnode->colour_code += next_colour_code;
    return 1;
}


int update_height(tree **stack, int stack_count){
    for (int i=stack_count-1; i > 0 ;i--){
        tree *node = stack[i];
        node->height += 1;
        int bf = get_bf(node->l, node->r);
        if (bf > 1 || bf < -1){
            return i;
        }
    }
    return -1;
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

    return rval - lval;
}
