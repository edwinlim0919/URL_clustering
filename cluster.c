#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dict.h"


void DFS_search(struct node *root, char URL[MAX_URL_LENGTH]) {
    struct node *curr_node = root;
    bool is_leaf = true;

    for (int i = 0; i < HASHSIZE; i++) {
        char URL_copy[MAX_URL_LENGTH] = "";
        strcpy(URL_copy, URL);
        struct nlist *child = curr_node->hashtab[i];
        if (child) {
            is_leaf = false;
        }

        while (child) {
            strcat(URL_copy, child->name);
            DFS_search(child->defn, URL_copy);

            child = child->next;
        }
    }

    if (is_leaf) {
        // print out string
        printf("%s\n", URL);
    }
}


void cluster_urls(char *url_filename) {
    FILE *url_file = fopen(url_filename, "r");

    if (url_file == NULL) {
        printf("Error: could not open file %s", url_filename);
    }

    struct node *root = init_node();
    char buffer[MAX_URL_LENGTH];

    while (fgets(buffer, MAX_URL_LENGTH, url_file)) {
        char *token = strtok(buffer, "/");
        char *schema = token;
        struct node *curr_node = root;

        while (token != NULL) {
            token = strtok(NULL, "/");

            if (token) {
                struct nlist *value = lookup(token, curr_node->hashtab);
                if (value == NULL) {
                    // doesn't exist in dictionary yet
                    struct node *new_node = init_node();
                    install(token, new_node, curr_node->hashtab);
                    curr_node = new_node;
                } else {
                    curr_node = value->defn;
                }
            }

            // printf("%s\n", token);
        }

        // printf("SCHEMA IS: %s\n", schema);
        // printf("\n\n");
    }

    char empty_URL[MAX_URL_LENGTH] = "";
    DFS_search(root, empty_URL);

    printf("\n");
    fclose(url_file);
    return;
}


void test_dictionary() {
    struct node *test_node = init_node();
    test_node->num_children = 4;
    install("test_node", test_node, test_node->hashtab);
    struct nlist *result = lookup("test_node", test_node->hashtab);

    if (result == NULL) {
        printf("result is NULL\n");
    } else {
        printf("result is not NULL\n");
        struct node *result_node = result->defn;
        printf("num_children of result_node: %d\n", result_node->num_children);
    }
}


int main(void) {
    cluster_urls("URLS0.txt");
    test_dictionary();
    return 0;
}