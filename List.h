#ifndef LIST
#define LIST
#include <stdint.h>
#include <Allocator.h>
typedef struct ListNode_t
{
	struct ListNode_t *next;
	void *content;
} ListNode;
typedef struct
{
	Allocator *allocator;
	ListNode *head;
	uint32_t length;
} List;
List createList( Allocator* );
void destroyList( List );
void appendBackList( List , void * );
void removeFirstWithContentList( List * , void * );
void forEachList( List , void ( * )( void * ) );
#endif

