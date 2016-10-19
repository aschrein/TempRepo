#ifndef DUMMY_LIST
#define DUMMY_LIST
#include <stdint.h>
typedef struct ListNode_t
{
	struct ListNode_t *next;
	void *content;
} ListNode;
//used for node or auxiliary allocation
typedef struct
{
	void *( *alloc )( size_t );
	void ( *free )( void * );
} Allocator;
typedef struct
{
	Allocator allocator;
	ListNode *head;
	unsigned int length;
} List;
typedef enum
{
	SUCCESS ,
	INVALID_ALLOCATOR ,
	ALLOC_ERROR ,
	NOT_FOUND_CONTENT ,
	INVALID_LIST ,
	INVALID_CONTENT
} Error;
typedef enum
{
	CONTINUE ,
	BREAK
} Flow;
Error createList( List **plist , Allocator allocator );
Error destroyList( List **plist );
//allocates new node and adds it to lists tail, increments lists length on success
Error appendBack( List *list , void *content );
//removes node with content pointer equal to passed and decrements lists length on success
Error removeFirstWithContent( List *list , void *content );
Error forEach( List *list , Flow ( *func )( void * ) );
#endif// DUMMY_LIST

