#include "DummyList.h"
#include <stdlib.h>
static ListNode *createNode( void *content , Allocator allocator )
{
	ListNode *out = allocator.alloc( sizeof( ListNode ) );
	if( out == NULL )
	{
		return NULL;
	}
	out->content = content;
	out->next = NULL;
	return out;
}
static void destroyNode( ListNode *node , Allocator allocator )
{
	allocator.free( node );
}
Error createList( List **plist , Allocator allocator )
{
	if( allocator.alloc == NULL || allocator.free == NULL )
	{
		return INVALID_ALLOCATOR;
	}
	List *out = allocator.alloc( sizeof( List ) );
	if( out == NULL )
	{
		return ALLOC_ERROR;
	}
	out->allocator = allocator;
	out->head = NULL;
	out->length = 0;
	*plist = out;
	return SUCCESS;
}
Error destroyList( List **plist )
{
	if( plist == NULL || *plist == NULL )
	{
		return INVALID_LIST;
	}
	List *list = *plist;
	if( list->head != NULL )
	{
		ListNode *cur_node = list->head;
		while( cur_node != NULL )
		{
			ListNode *next = cur_node->next;
			destroyNode( cur_node , list->allocator );
			cur_node = next;
		}
	}
	list->allocator.free( list );
	*plist = NULL;
	return SUCCESS;
}
static ListNode *findTail( ListNode *head )
{
	if( head == NULL )
	{
		return NULL;
	}
	ListNode *cur_node = head;
	while( cur_node->next != NULL )
	{
		cur_node = cur_node->next;
	}
	return cur_node;
}
Error appendBack( List *list , void *content )
{
	if( list == NULL )
	{
		return INVALID_LIST;
	}
	if( content == NULL )
	{
		return INVALID_CONTENT;
	}
	if( list->head == NULL )
	{
		list->head = createNode( content , list->allocator );
		if( list->head == NULL )
		{
			return ALLOC_ERROR;
		}
		list->length = 1;
		return SUCCESS;
	}
	ListNode *tail = findTail( list->head );
	tail->next = createNode( content , list->allocator );
	if( tail->next == NULL )
	{
		return ALLOC_ERROR;
	}
	list->length++;
	return SUCCESS;
}
Error removeFirstWithContent( List *list , void *content )
{
	if( list == NULL || list->head == NULL )
	{
		return INVALID_LIST;
	}
	if( list->head->content == content )
	{
		ListNode *next = list->head->next;
		destroyNode( list->head , list->allocator );
		list->head = next;
		list->length--;
		return SUCCESS;
	}
	ListNode *cur_node = list->head;
	while( cur_node->next != NULL )
	{
		if( cur_node->next->content == content )
		{
			ListNode *next = cur_node->next->next;
			destroyNode( cur_node->next , list->allocator );
			cur_node->next = next;
			list->length--;
			return SUCCESS;
		}
		cur_node = cur_node->next;
	}
	return NOT_FOUND_CONTENT;
}
Error forEach( List *list , Flow ( *func )( void * ) )
{
	if( list == NULL || list->head == NULL )
	{
		return INVALID_LIST;
	}
	ListNode *cur_node = list->head;
	while( cur_node != NULL )
	{
		if( func( cur_node->content ) == BREAK )
		{
			return SUCCESS;
		}
		cur_node = cur_node->next;
	}
	return SUCCESS;
}
