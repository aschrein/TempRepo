#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
typedef struct
{
	int user_id;
	int len;
	char raw[ 0 ];
} MessageNode;
typedef struct
{
	sem_t semaphore;
	MessageNode nodes[];
} MessageBoard;
#define SHARED_MEM_SIZE 0x100000
#define USER_LIST_OFFSET 0xf0000
typedef struct
{
	int count;
	int ids[ 0 ];
} UserList;
UserList *getUserList( MessageBoard *msg_board )
{
	return ( char* )msg_board + USER_LIST_OFFSET;
}
int addUser( UserList *ulist )
{
	int i;
	for( i = 0; i < ulist->count; i++ )
	{
		if( ulist->ids[ i ] < 0 )
		{
			ulist->ids[ i ] = 1;
			return i;
		}
	}
	ulist->ids[ ulist->count++ ] = 1;
	return ulist->count - 1;
}
void removeUser( UserList *ulist , int id )
{
	ulist->ids[ id ] = -1;
	while( ulist->count && ulist->ids[ ulist->count - 1 ] < 0 ) ulist->count--;
}
void addMessage( MessageBoard *board , int user_id , char *raw_msg )
{
	sem_wait( &board->semaphore );
	int offset = 0;
	MessageNode *cur = board->nodes;
	while( cur->len > 0 )
	{
		cur = &cur->raw[ 0 ] + cur->len;
	}
	sprintf( cur->raw , "%s" , raw_msg );
	cur->user_id = user_id;
	cur->len = strlen( cur->raw ) + 1;
	cur = &cur->raw[ 0 ] + cur->len;
	cur->len = -1;
	sem_post( &board->semaphore );
}