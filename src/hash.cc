#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct nlist {          /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name;         /* defined name */
    int *count;         /* replacement text */
};

#define HASHSIZE 101

unsigned hash(char *);
struct nlist *lookup(char *s);
struct nlist *install(char *, char *);
void undef(char *);

static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval & HASHSIZE;
}

/* lookup: look for s in hashtab */
struct nlist *lookup(char *s)
{
    struct nlist *np;

    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
            return np;    /* found */
    return NULL;          /* not found */
}

struct nlist *lookup(char *s);
char *strdup(const char *);

struct nlist *install(char *name)
{
    struct nlist *np;
    unsigned hashval;

    if ((np = lookup(name)) == NULL) {    /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
            return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
	// *np->count = 0;
        hashtab[hashval] = np;
    } else        /* already there */
        free((void *) np->count);
    return np;
}

void undef(char *name)
{
    struct nlist *np, *prev;
    unsigned hashval;
    prev = NULL;
    hashval = hash(name);

    for (np = hashtab[hashval]; np != NULL; np = np->next) {
        if (strcmp(name, np->name) == 0)
            break;
        prev = np;
    }
    if (np != NULL) {
        if (prev == NULL) {
            hashtab[hashval] = np->next; // first on the list, put np->next on top
        } else {
            prev->next = np->next; // somewhere in middle, patch list
        }
        free((void *) np->name);
        free((void *) np->count);
        free((void *) np);
    }
}
