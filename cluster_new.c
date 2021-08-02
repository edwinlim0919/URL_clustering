#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
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


struct node {
  char *name;
  char **children;
  int num_children;
  int max_children;
  DICTIONARY *dict;
};


struct node *init_node(char *name) {
  struct node *new_node = (struct node*) malloc(sizeof(struct node));
  if (new_node == NULL) {
    printf("Failed to malloc new_node in INIT_NODE.\n");
    return NULL;
  }

  new_node->name = name;
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


void resize_children(struct node *resize_node) {
  resize_node->max_children = resize_node->max_children * 2;
  resize_node->children = (char**) realloc(resize_node->children, sizeof(char*) * resize_node->max_children);
  if (resize_node->children == NULL) {
    printf("Failed to realloc resize_node->children in RESIZE_CHILDREN.\n");
  }
}


void add_child(struct node *parent, struct node *child) {
  if (parent->num_children == parent->max_children) {
    resize_children(parent);
  } 

  DAdd(parent->dict, child, child->name);
  parent->children[parent->num_children] = child->name;
  parent->num_children++;
}


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

      while (token != NULL) {
          token = strtok(NULL, "/");

          if (token) {
              token[strcspn(token, "\n")] = 0;
              struct node *value = GetDataWithKey(curr_node->dict, token);

              if (value == NULL) {
                // Create new dictionary child
                printf("CREATING NEW NODE %s FOR %s\n", token, curr_node->name);
        
                struct node *child = init_node(token);
                add_child(curr_node, child);
                curr_node = child;
              } else {
                // Use existing child
                printf("USING EXISTING NODE %s FROM %s\n", value->name, curr_node->name);

                curr_node = value;
              }

              // printf("%s\n", token);
          }
      }

      printf("SCHEMA IS: %s\n", schema);
      printf("\n\n");
  }

  char empty_URL[MAX_URL_LENGTH] = "";

  printf("\n");
  fclose(url_file);
  return;
}


void test_dictionary() {
  DICTIONARY *new_dict = InitDictionary();
  DAdd(new_dict, "value", "key");
  char *value = GetDataWithKey(new_dict, "key");
  printf("VALUE: %s\n", value);
}


int main(void) {
  cluster_urls("URLS0.txt");
  test_dictionary();
  return 0;
}