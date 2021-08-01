#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dict.h"


void DFS_debug(struct node *curr_node) {
    for (int i = 0; i < HASHSIZE; i++) {
        struct nlist *child = curr_node->hashtab[i];
        
        while (child) {
            printf("%s\n", child->name);
            DFS_debug(child->defn);
            child = child->next;
        }
    }
}


void DFS_search(struct node *curr_node, char URL[MAX_URL_LENGTH]) {
    bool is_leaf = true;

    for (int i = 0; i < HASHSIZE; i++) {
        struct nlist *child = curr_node->hashtab[i];
        if (child) {
            is_leaf = false;
        }

        while (child) {
            char URL_copy[MAX_URL_LENGTH] = "";
            strcpy(URL_copy, URL);
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

    struct node *root = init_node("root");
    char buffer[MAX_URL_LENGTH];

    while (fgets(buffer, MAX_URL_LENGTH, url_file)) {
        char *token = strtok(buffer, "/");
        char *schema = token;
        struct node *curr_node = root;
        int depth = 0;

        while (token != NULL) {
            token = strtok(NULL, "/");

            if (token) {
                token[strcspn(token, "\n")] = 0;
                printf("%s: %d\n", token, depth);
                struct nlist *value = lookup(token, curr_node->hashtab);

                if (value == NULL) {
                    // doesn't exist in dictionary yet
                    printf("CREATING NEW NODE %s FOR %s\n", token, curr_node->name);
                    struct node *new_node = init_node(token);
                    install(token, new_node, curr_node->hashtab);
                    curr_node = new_node;
                } else {
                    printf("USING EXISTING NODE %s FROM %s\n", value->name, curr_node->name);
                    curr_node = value->defn;
                }

                depth++;
            }
        }

        // printf("SCHEMA IS: %s\n", schema);
        printf("\n");
    }

    char empty_URL[MAX_URL_LENGTH] = "";
    // DFS_search(root, empty_URL);
    // DFS_debug(root);

    printf("\n");
    fclose(url_file);
    return;
}


void test_dictionary() {
    struct node *test_node = init_node("");
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