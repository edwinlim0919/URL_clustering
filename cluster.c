#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"


void add_root_child(int *num_root_children, int *max_num_root_children, struct node *root) {
    if ((*num_root_children) == (*max_num_root_children)) {
        // realloc data and add new child node, update ints
        
    } else {
        // add child node, update ints

    }
    return;
}


void cluster_urls(char *url_filename) {
    FILE *url_file = fopen(url_filename, "r");

    if (url_file == NULL) {
        printf("Error: could not open file %s", url_filename);
    }

    // root starts off w/ 32 children, will realloc as it grows
    int num_root_children = 0;
    int max_num_root_children = 32;
    struct node *root = malloc(max_num_root_children * sizeof(struct node*));
    char buffer[MAX_URL_LENGTH];

    while (fgets(buffer, MAX_URL_LENGTH, url_file)) {
        char *token = strtok(buffer, "/");

        while (token != NULL) {
            printf("%s\n", token);
            token = strtok(NULL, "/");
        }

        printf("\n");
    }

    printf("\n");
    fclose(url_file);
    return;
}


void test_dictionary() {
    install("key", "value");
}


int main(void) {
    cluster_urls("URLS0.txt");
    return 0;
}