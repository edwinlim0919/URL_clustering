#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const unsigned MAX_URL_LENGTH = 1024;
const unsigned MAX_CHILDREN = 50;


struct node {
    struct node *children[MAX_CHILDREN];
    int num_children;
};


struct node *init_node() {
    struct node *new_node = (node *) malloc(sizeof(struct node));
    new_node->num_children = 0;

    return new_node;
}


struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* defined name */
    char *defn; /* replacement text */
};


#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */


/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}


/* lookup: look for s in hashtab */
struct nlist *lookup(char *s)
{
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}


char *strdup_dict(char *);
/* install: put (name, defn) in hashtab */
struct nlist *install(char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup_dict(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdup_dict(defn)) == NULL)
       return NULL;
    return np;
}


char *strdup_dict(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}