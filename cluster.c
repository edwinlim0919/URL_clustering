#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdbool.h>
#include "header.h"
#include "dictionary.h"


/* hash: form hash value for string s */
int make_hash(char *s)
{
    int hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % (MAX_HASH_SLOT - 1);
}


DICTIONARY* InitDictionary() {
  DICTIONARY* d = (DICTIONARY*)malloc(sizeof(DICTIONARY));
  MALLOC_CHECK(d);
  d->start = d->end = NULL;
  int i;
  for (i = 0; i < MAX_HASH_SLOT; i++) {
    d->hash[i] = NULL;
  }
  return d;
}


void CleanDictionary(DICTIONARY* dict) {
  DNODE* d = dict->start;
  while (d != NULL) {
    DNODE* dd = d;
    if (dd->data != NULL)
      free(dd->data);
    d = d->next;
    free(dd);
  }
}


void DAdd(DICTIONARY* dict, void* data, char* key) {
  MYASSERT(strlen(key) > 0);
  int h = make_hash(key);
  if (dict->start == NULL) {
    //! The first element of our dictionary
    //! I assume \a dict->end is also NULL
    MYASSERT(dict->end == NULL);
    dict->start = dict->end = (DNODE*)malloc(sizeof(DNODE));
    MALLOC_CHECK(dict->start);
    dict->start->prev = dict->start->next = NULL; 
    dict->hash[h] = dict->start;
    dict->start->data = data;
    BZERO(dict->start->key, KEY_LENGTH);
    strncpy(dict->start->key, key, KEY_LENGTH);
  } else {
    DNODE* d;
    //! Not the first element.
    if (dict->hash[h]) {
      //! Same hash.
      int flag = 0; //!< zero the key is not in our dictionary, one if does
      //! First, we want to see is this key already in our dictionary
      for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
           d = d->next) {
        if (!strncmp(d->key, key, KEY_LENGTH)) {
          //! We find the same key.
          if (d->data != NULL)
            free(d->data);
          d->data = data;
          flag = 1;
          break;
        }        
      }
      //! not in.
      if (flag == 0) {
        //! We didin't find the same key.
        if (d == NULL)
          d = dict->end;
        else
          d = d->prev;
        DNODE* dd = (DNODE*)malloc(sizeof(DNODE));
        MALLOC_CHECK(dd);
        dd->next = d->next;
        dd->prev = d;
        if (dd->next == NULL)
          dict->end = dd;
        d->next = dd;
        BZERO(dd->key, KEY_LENGTH);
        strncpy(dd->key, key, KEY_LENGTH);
        dd->data = data;
      }
     } else {
      //! No same hash, create at the end of the list.
      DNODE* d = (DNODE*)malloc(sizeof(DNODE));
      d->next = NULL;
      d->prev = dict->end;
      dict->end->next = d;
      //! add to our cash slot
      dict->hash[h] = d;
      d->data = data;
      BZERO(d->key, KEY_LENGTH);
      strncpy(d->key, key, KEY_LENGTH);
      //! change the end of our list
      dict->end = d;
    }
  }
}


void DRemove(DICTIONARY* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  if (dict->hash[h] == NULL)
    return;
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) {
    //! OK, we find our node.
    if (!strncmp(d->key, key, KEY_LENGTH)) {
      //! is it the last node.
      if (d->next == NULL) 
        dict->end = d->prev;
      else
         d->next->prev = d->prev;
      //! or is it the first node.
      if (d->prev == NULL)
        dict->start = d->next;
      else
        d->prev->next = d->next;
      //! update hash slot, 
      if (dict->hash[h] == d) {
        if ((d->next != NULL)&&(make_hash(d->next->key)==h)) {
          dict->hash[h] = d->next;
        } else {
          dict->hash[h] = NULL;
        }
      }
      if (d->data != NULL)
        free(d->data);
      free(d);
    }
  }
}


void* GetDataWithKey(DICTIONARY* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  //! This speed up the process.
  if (dict->hash[h] == NULL)
    return NULL;
  //! ok, we have the hash, so we find the actual key.
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) 
    if (!strncmp(d->key, key, KEY_LENGTH)) 
      return d->data;
  return NULL;
}


DNODE* GetDNODEWithKey(DICTIONARY* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  //! This speed up the process.
  if (dict->hash[h] == NULL)
    return NULL;
  //! ok, we have the hash, so we find the actual key.
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) 
    if (!strncmp(d->key, key, KEY_LENGTH)) 
      return d;
  return NULL;
}


// Nodes that make up the URL tree
struct node {
  char *name;
  char **children;
  int num_children;
  int max_children;
  DICTIONARY *dict;
};


// Initializes a URL tree node given a string
struct node *init_node(char *name) {
  struct node *new_node = (struct node*) malloc(sizeof(struct node));
  if (new_node == NULL) {
    printf("Failed to malloc new_node in INIT_NODE.\n");
    return NULL;
  }

  new_node->name = (char*) malloc(sizeof(char) * MAX_URL_LENGTH);
  strcpy(new_node->name, name);
  new_node->num_children = 0;
  new_node->max_children = 32;
  new_node->dict = InitDictionary();
  new_node->children = (char**) malloc(sizeof(char*) * new_node->max_children);
  if (new_node->children == NULL) {
    printf("Failed to malloc new_node->children in INIT_NODE.\n");
    return NULL;
  }

  return new_node;
}


// Reallocs more space for children when node runs out of room
void resize_children(struct node *resize_node) {
  resize_node->max_children = resize_node->max_children * 2;
  resize_node->children = (char**) realloc(resize_node->children, sizeof(char*) * resize_node->max_children);
  if (resize_node->children == NULL) {
    printf("Failed to realloc resize_node->children in RESIZE_CHILDREN.\n");
  }
}


// Adds the given child node to the given parent node
void add_child(struct node *parent, struct node *child) {
  if (parent->num_children == parent->max_children) {
    resize_children(parent);
  } 

  DAdd(parent->dict, child, child->name);
  parent->children[parent->num_children] = child->name;
  parent->num_children++;
}


// For debugging individual nodes of the URL tree
void DFS_debug(struct node *curr_node) {
  for (int i = 0; i < curr_node->num_children; i++) {
    char *child_key = curr_node->children[i];
    printf("%s\n", child_key);
  }
}


// Prints out all URL paths stemming from current node
void DFS_search(struct node *curr_node, char URL[MAX_URL_LENGTH]) {
  if (curr_node == NULL) {
    return;
  } else if (curr_node->num_children == 0) {
    printf("%s\n", URL);
  }

  for (int i = 0; i < curr_node->num_children; i++) {
    char *child_key = curr_node->children[i];
    struct node *child = GetDataWithKey(curr_node->dict, child_key);
    char URL_copy[MAX_URL_LENGTH] = "";
    strcpy(URL_copy, URL);
    strcat(URL_copy, child_key);
    DFS_search(child, URL_copy);
  }
}


// Counts the number of URL paths stemming from the current node
int DFS_enumerate(struct node *curr_node) {
  if (curr_node == NULL) {
    return 0;
  } else if (curr_node->num_children == 0) {
    return 1;
  }

  int curr_count = 0;

  for (int i = 0; i < curr_node->num_children; i++) {
    char *child_key = curr_node->children[i];
    struct node *child = GetDataWithKey(curr_node->dict, child_key);
    curr_count += DFS_enumerate(child);
  }

  return curr_count;
}


// Populates list of URL's with all URL paths stemming from current node
int DFS_find_URLs(struct node *curr_node, int URL_index, char **URL_list, char URL[MAX_URL_LENGTH]) {
  if (curr_node == NULL) {
    return URL_index;
  } else if (curr_node->num_children == 0) {
    *(URL_list + URL_index) = (char *) malloc(MAX_URL_LENGTH * sizeof(char));
    strcpy(*(URL_list + URL_index), URL);
    return URL_index + 1;
  }

  for (int i = 0; i < curr_node->num_children; i++) {
    char *child_key = curr_node->children[i];
    struct node *child = GetDataWithKey(curr_node->dict, child_key);
    char URL_copy[MAX_URL_LENGTH] = "";
    strcpy(URL_copy, URL);

    if (strcmp(URL_copy, "") != 0) {
      strcat(URL_copy, "/");
    }

    strcat(URL_copy, child_key);
    URL_index = DFS_find_URLs(child, URL_index, URL_list, URL_copy);
  }

  return URL_index;
}


// Returns -1 if:
//    URLs are of different length
//    URLs differ in more than one index
// Else:
//    Returns index at which URLs differ
int compare_URLs(char URL1[MAX_URL_LENGTH], char URL2[MAX_URL_LENGTH]) {
  char *save_ptr1, *save_ptr2;
  char *token1 = strtok_r(URL1, "/", &save_ptr1);
  char *token2 = strtok_r(URL2, "/", &save_ptr2);
  int diff_count = 0;
  int curr_index = 0;
  int diff_index = 0;

  while (token1 != NULL && token2 != NULL) {

    if (strcmp(token1, token2) != 0) {
      diff_index = curr_index;
      diff_count++;
    }

    token1 = strtok_r(NULL, "/", &save_ptr1);
    token2 = strtok_r(NULL, "/", &save_ptr2);
    curr_index++;
  }

  if ((token1 && !token2) || (!token1 && token2) || (diff_count > 1)) {
    return -1;
  } else {
    return diff_index;
  }
}


// Prints all URLs pointed to by URL_list
void print_URLs(char *URL_list[MAX_URL_LENGTH], int num_URLs) {
  for (int i = 0; i < num_URLs; i++) {
    printf("%s\n", URL_list[i]);
  }
}


// Returns a list of URL indices if similarity threshold is reached, else NULL: O(n^3), n = # of URLs
int *find_similarity(char **URL_list, int num_URLs) {
  // Index list will store all indices at which URLs differ in only one position, will also hold where URLs differ in the last element
  int *index_list = (int *) calloc(SIMILARITY_THRESHOLD+1, sizeof(int));

  for (int i = 0; i < num_URLs; i++) {
    int curr_index = 0;
    *(index_list + curr_index) = i;
    curr_index++;

    for (int j = 0; j < num_URLs; j++) {
      // When prev_comparison == -2, that means we don't have an index value for it yet
      int prev_comparison = -2;
      int curr_index = 1;
      char buf1[MAX_URL_LENGTH] = "";
      char buf2[MAX_URL_LENGTH] = "";
      strcpy(buf1, *(URL_list + i));
      strcpy(buf2, *(URL_list + j));
      int URL_comparison = compare_URLs(buf1, buf2);

      if (i != j && URL_comparison != -1) {
        // The URLs are the same, except for the word at index URL_comparison, compare with the rest of the indices
        *(index_list + curr_index) = j;
        curr_index++;

        for (int j_new = j+1; j_new < num_URLs && j_new != i; j_new++) {
          if (prev_comparison == -2) {
            *(index_list + SIMILARITY_THRESHOLD) = URL_comparison;
            prev_comparison = URL_comparison;
            *(index_list + curr_index) = j_new;
            curr_index++;
          } else {

            if (URL_comparison == prev_comparison) {
              *(index_list + curr_index) = j_new;
              curr_index++;
            } else {
              continue;
            }
          }
        }

        if (curr_index == SIMILARITY_THRESHOLD) {
          return index_list;
        } else {
          free(index_list);
          index_list = (int *) calloc(SIMILARITY_THRESHOLD+1, sizeof(int));
        }
      }
    }
  }

  if (index_list) {
    free(index_list);
  }

  return NULL;
}


// Frees a tree starting from node curr_node
void free_tree(struct node *curr_node) {

  if (curr_node == NULL) {
    return;
  }

  for (int i = 0; i < curr_node->num_children; i++) {
    char *child_key = *(curr_node->children + i);
    struct node *child = (struct node *) GetDataWithKey(curr_node->dict, child_key);
    free_tree(child);
  }

  free(curr_node->children);
  CleanDictionary(curr_node->dict);
  free(curr_node->dict);
  free(curr_node->name);
  free(curr_node);
}


// Frees a branch of curr_node given a URL path
void free_branch(struct node *curr_node, char URL[MAX_URL_LENGTH]) {
  char *token = strtok(URL, "/");

  while (token != NULL) {
    printf("%s\n", token);
    token[strcspn(token, "\n")] = 0;
    

    token = strtok(NULL, "/");
  }
  printf("\n");
}


// Main algorithm to cluster a .txt file of URLs
void cluster_urls(char *url_filename) {
  FILE *url_file = fopen(url_filename, "r");

  if (url_file == NULL) {
      printf("Error: could not open file %s", url_filename);
  }

  char buffer[MAX_URL_LENGTH];
  struct node *root = init_node("root");

  while (fgets(buffer, MAX_URL_LENGTH, url_file)) {
      char *token = strtok(buffer, "/");
      char *schema = token;
      struct node *curr_node = root;
      struct node *fork_point = NULL;

      while (token != NULL) {
          token = strtok(NULL, "/");

          if (token) {
              token[strcspn(token, "\n")] = 0;
              struct node *value = GetDataWithKey(curr_node->dict, token);

              if (value == NULL) {
                // No URL match, create new dictionary child
                if (fork_point == NULL) {
                    fork_point = curr_node;
                }

                struct node *child = init_node(token);
                add_child(curr_node, child);
                curr_node = child;
              } else {
                // URL matches so far, use existing child
                curr_node = value;
              }
          }
      }

      if (fork_point) {
          // After URL is added, start clustering from fork point if URL is new
          int num_URLs = DFS_enumerate(fork_point);
          char **URL_list = (char **) malloc(sizeof(char *) * num_URLs);
          char empty_URL[MAX_URL_LENGTH] = "";
          DFS_find_URLs(fork_point, 0, URL_list, empty_URL);
          int *index_list = find_similarity(URL_list, num_URLs);

          if (index_list) {
            printf("SIMILARITY EXISTS!\n");

            int diff_index = *(index_list + SIMILARITY_THRESHOLD);

            // Delete duplicate branches
            for (int i = 0; i < SIMILARITY_THRESHOLD; i++) {
              int URL_list_index = *(index_list + i);
              char *URL = *(URL_list + URL_list_index);
              char *URL = URL_list[URL_list_index];
              printf("%s\n", URL);

              // free_branch(fork_point, URL);
            }

            // Replace remaining branch param at diff index with {arg}
          }
      }
  }

  int num_URLs_total = DFS_enumerate(root);
  printf("num_URLs_total: %d\n", num_URLs_total);
  char **URL_list_total = (char **) malloc(sizeof(char *) * num_URLs_total);
  char empty_URL[MAX_URL_LENGTH] = "";
  DFS_find_URLs(root, 0, URL_list_total, empty_URL);
  print_URLs(URL_list_total, num_URLs_total);

  fclose(url_file);
  return;
}


// Basic dictionary test
void test_dictionary() {
  DICTIONARY *new_dict = InitDictionary();
  DAdd(new_dict, "value", "key");
  char *value = GetDataWithKey(new_dict, "key");
  printf("VALUE: %s\n", value);
}


// URL comparison test
void test_URL_comparison() {
  // Different at index 1
  char URL1[MAX_URL_LENGTH] = "first/a/third";
  char URL2[MAX_URL_LENGTH] = "first/b/third";
  int result = compare_URLs(URL1, URL2);
  printf("Result should be: 1 and is: %d\n", result);

  // Different in two locations
  char URL3[MAX_URL_LENGTH] = "first/a/third";
  char URL4[MAX_URL_LENGTH] = "first/b/notthird";
  result = compare_URLs(URL3, URL4);
  printf("Result should be: -1 and is: %d\n", result);

  // Different lengths
  char URL5[MAX_URL_LENGTH] = "first/a/third";
  char URL6[MAX_URL_LENGTH] = "first/b";
  result = compare_URLs(URL5, URL6);
  printf("Result should be: -1 and is: %d\n", result);

  // Different at index 0
  char URL7[MAX_URL_LENGTH] = "a/second/third";
  char URL8[MAX_URL_LENGTH] = "b/second/third";
  result = compare_URLs(URL7, URL8);
  printf("Result should be: 0 and is: %d\n", result);

  // Different at index 3
  char URL9[MAX_URL_LENGTH] = "first/second/third/a";
  char URL10[MAX_URL_LENGTH] = "first/second/third/b";
  result = compare_URLs(URL9, URL10);
  printf("Result should be: 3 and is: %d\n", result);
}


int main(void) {
  // cluster_urls("URLS0.txt");
  // printf("\n\n\n");
  // test_URL_comparison();
  cluster_urls("URLS1.txt");
  return 0;
}