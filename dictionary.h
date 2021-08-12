#ifndef _DICTIONARY_MAP_
#define _DICTIONARY_MAP_
#define KEY_LENGTH 1000
#define MAX_URL_LENGTH 1024
#define MAX_HASH_SLOT 10000
#define SIMILARITY_THRESHOLD 10


typedef struct _DNODE {
  struct _DNODE* next;
  struct _DNODE* prev;
  void* data; //!< actual data.
  char key[KEY_LENGTH]; //!< actual key.
} __DNODE;


typedef struct _DNODE DNODE;


typedef struct _DICTIONARY {
  DNODE* hash[MAX_HASH_SLOT]; // the hash table of slots, each slot points to a DNODE
  DNODE* start; // start of double link list of DNODES terminated by NULL pointer
  DNODE* end;  // points to the last DNODE on this list
} __DICTIONARY;


typedef struct _DICTIONARY DICTIONARY;
DICTIONARY* InitDictionary();
void CleanDictionary(DICTIONARY* dict);
void DAdd(DICTIONARY* dict, void* data, char* key);
void DRemove(DICTIONARY* dict, char* key);
void* GetDataWithKey(DICTIONARY* dict, char* key);
DNODE* GetDNODEWithKey(DICTIONARY* dict, char* key);
int make_hash(char* c);


#endif