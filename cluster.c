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
  char *token1 = strtok(URL1, "/");
  char *token2 = strtok(URL2, "/");
  

  while (token1 != NULL && token2 != NULL) {

  }
}


// Prints all URLs pointed to by URL_list
void print_URLs(char *URL_list[MAX_URL_LENGTH], int num_URLs) {
  for (int i = 0; i < num_URLs; i++) {
    printf("%s\n", URL_list[i]);
  }
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
                    printf("RECORDING FORK POINT: %s\n", curr_node->name);
                }

                printf("CREATING NEW NODE %s FOR %s\n", token, curr_node->name);
                struct node *child = init_node(token);
                add_child(curr_node, child);
                curr_node = child;
              } else {
                // URL matches so far, use existing child
                printf("USING EXISTING NODE %s FROM %s\n", value->name, curr_node->name);
                curr_node = value;
              }
          }
      }

      if (fork_point) {
          // After URL is added, start clustering from fork point if URL is new
          int num_URLs = DFS_enumerate(fork_point);

      }

      printf("SCHEMA IS: %s\n", schema);
      printf("\n\n");
  }

  printf("\n\n\n");
  char empty_URL1[MAX_URL_LENGTH] = "";
  DFS_search(root, empty_URL1);
  printf("\n\n\n");
  int URL_COUNT = DFS_enumerate(root);
  printf("URL COUNT: %d\n", URL_COUNT);
  printf("\n\n\n");
  char **URL_list = (char **) malloc(sizeof(char *) * URL_COUNT);
  char empty_URL2[MAX_URL_LENGTH] = "";
  DFS_find_URLs(root, 0, URL_list, empty_URL2);
  print_URLs(URL_list, URL_COUNT);
  printf("\n\n\n");

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


int main(void) {
  cluster_urls("URLS0.txt");
  return 0;
}