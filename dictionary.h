//! A dictionary-like data structure based on hash functions.
#ifndef _DICTIONARY_MAP_
#define _DICTIONARY_MAP_


// Set some magic numbers. These are large values.

// Unlikely to have more than an a URL longer that 1000 chars.
// The KEY is the same as a URL. The term KEY is just a
// general Dictionary/hash function term
#define KEY_LENGTH 1000
#define MAX_URL_LENGTH 1024
#define MAX_HASH_SLOT 10000


//! Dictionary Node
// Dictionary Node. This is a general double link list structure that
// holds the key (URL - we explained into today's lecture why this is there)
// and a pointer to void that points to a URLNODE in practice.
// key is the same as URL recall.
typedef struct _DNODE {
  struct _DNODE* next;
  struct _DNODE* prev;
  void* data; //!< actual data.
  char key[KEY_LENGTH]; //!< actual key.
} __DNODE;

typedef struct _DNODE DNODE;


// The DICTIONARY holds the hash table and the start and end pointers into a double
// link list. This is a unordered list with the exception that DNODES with the same key (URL)
// are clusters along the list. So you hash into the list. Check for uniqueness of the URL.
// If not found add to the end of the cluster associated wit the same URL. You will have
// to write an addElement function.
typedef struct _DICTIONARY {
  DNODE* hash[MAX_HASH_SLOT]; // the hash table of slots, each slot points to a DNODE
  DNODE* start; // start of double link list of DNODES terminated by NULL pointer
  DNODE* end;  // points to the last DNODE on this list
} __DICTIONARY;

typedef struct _DICTIONARY DICTIONARY;

// \brief call to create a new \a DICTIONARY object
DICTIONARY* InitDictionary();

// \call to free a new \a DICTIONARY object
void CleanDictionary(DICTIONARY* dict);

//! \brief add a new (key, data) pair into the dictionary.
//!
//! \param dict the dictionary you initialized by InitDictionary()
//! \param data the data for the key
//! \param key a string
//! If the key already exists, it will free() the original
//! \a data and replace with the new \a data
//! \warning the program will run free() on each \a data
//!          when you call \a CleanDictionary()
void DAdd(DICTIONARY* dict, void* data, char* key);

//! \brief remove a (key, data) pair and free() the data.
//!        do nothing if there is no such key.
void DRemove(DICTIONARY* dict, char* key);

//! \brief use the key to retrieve the data.
//!
//! \return NULL if not found, otherwise the \a data
void* GetDataWithKey(DICTIONARY* dict, char* key);

//! \brief use the key to retrieve the DNODE
//!
//! \return NULL if not found, otherwise the \a DNODE
DNODE* GetDNODEWithKey(DICTIONARY* dict, char* key);


//! \brief generate a hash value h, and h=h%MAX_HASH_SLOT
//!
//! \param c the string 
//! \return the h
int make_hash(char* c);

#endif