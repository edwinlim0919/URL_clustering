#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASHSIZE 101
#define MAX_URL_LENGTH 1024


struct nlist;
struct node {
    struct nlist *hashtab[HASHSIZE];
    int num_children;
};


struct node *init_node() {
    struct node *new_node = (struct node*) malloc(sizeof(struct node));
    if (new_node != NULL) {
        new_node->num_children = 0;
    }

    return new_node;
}


struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* key */
    struct node *defn; /* value */
};


/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}


/* lookup: look for s in hashtab */
struct nlist *lookup(char *s, struct nlist *hashtab[HASHSIZE])
{
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}


struct node *strdup_dict_node(struct node *);
char *strdup_dict(char *);
/* install: put (name, defn) in hashtab */
struct nlist *install(char *name, struct node *defn, struct nlist *hashtab[HASHSIZE])
{
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(name, hashtab)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup_dict(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdup_dict_node(defn)) == NULL)
       return NULL;
    return np;
}


void copy_node(struct node *dest, struct node *src) {
    dest->num_children = src->num_children;
    for (unsigned i = 0; i < HASHSIZE; i++) {
        dest->hashtab[i] = src->hashtab[i];
    }
}


struct node *strdup_dict_node(struct node *s) /* make a duplicate of s */
{
    struct node *p;
    p = init_node();
    if (p != NULL)
       copy_node(p, s);
    return p;
}


char *strdup_dict(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}