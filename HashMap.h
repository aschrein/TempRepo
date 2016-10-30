#ifndef HASHMAP_H
#define HASHMAP_H
#include <stdint.h>
#include <stdlib.h>
#include <List.h>
typedef struct HashMapNode_t
{
	void *key;
	void *value;
	uint32_t hash;
} HashMapNode;
typedef struct
{
	Allocator *allocator;
	List *list;
} HashMap;
typedef uint32_t ( * )( void * ) HashFunc;
typedef uint32_t ( * )( void * , void * ) CompFunc;
HashMap createHashMap( Allocator * );
HashMap destroyHashMap( HashMap );
void addToHashMap( HashMap , void *key , void *value , HashFunc , CompFunc );
void removeFromHashMap( HashMap , void *key , HashFunc , CompFunc );
void containsHashMap( HashMap , void *key , HashFunc , CompFunc );
#endif
