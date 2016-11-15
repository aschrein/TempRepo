#define _GNU_SOURCE
static inline int strCmp( char const *a , char const *b ){ while( *a != '\0' && *a++ == *b++ ); return *a == *b && *b == '\0'; }
static inline void strcopy( char *dst , char const *src ){ while( *dst++ = *src++ ); }
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdint.h>
//using GNU GCC Built-in functions for atomic op; clone and futexes from linux; may not work on something that is not linux x86_64;
typedef struct
{
	void *stack;
	size_t stack_size;
	int th_pid;
} Task;
Task *createTask( void *routine , size_t stack_size )
{
	Task *task = ( Task* )malloc( sizeof( Task ) );
	task->stack_size = stack_size;
	task->stack = mmap( NULL , stack_size , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS  , -1 , 0 );
	task->th_pid = clone( routine , task->stack + stack_size - 1 , CLONE_VM | CLONE_FS , task );
	return task;
}
void destroyTask( Task *task )
{
	waitpid( task->th_pid , NULL , 0 );
	munmap( task->stack , task->stack_size );
	free( task );
}
typedef struct
{
	int product_count;
	int use_count;
	uint32_t lock;
} StoreHouse;
//lock and unlock routines peeked from man lock; atomic compare exchange and then wait on lock; SYS_futex can false wake;
//seems like if there is a lot of customers in the queue, loader can never wake and load becouse there is no guarantees about who wakes up next
//good realization needs queues with wake guarantees based on order
int lockStore( StoreHouse *store , int wait )
{
	if( wait )
	{
		while( !__sync_bool_compare_and_swap( &store->lock , 1 , 0 ) )
		{
			//sleeps if lock is still 0
			syscall( SYS_futex , &store->lock , FUTEX_WAIT , 0 , NULL , NULL , 0 );
		}
		return 1;
	} else
	{
		return __sync_bool_compare_and_swap( &store->lock , 1 , 0 );
	}
}
//even if user did not wait to acuire lock, he still needs to fire event
void unlockStore( StoreHouse *store )
{
	if( __sync_bool_compare_and_swap( &store->lock , 0 , 1 ) )
	{
		//wake all waiters
		syscall( SYS_futex , &store->lock , FUTEX_WAKE , 0x7fffffff , NULL , NULL , 0 );
	}
}
#define NUM_STORE_HOUSES 3
StoreHouse store_houses[ NUM_STORE_HOUSES ];
#define NUM_CUSTOMERS 32
uint32_t customers_unpleased;
Task *customers[ NUM_CUSTOMERS ];
Task *loader;
int chooseStoreHouse()
{
	return rand() % NUM_STORE_HOUSES;
}
int customerRoutine( Task *task )
{
	srand( clock() );
	int needs = 1000;
	while( needs > 0 )
	{
		int current_needs = rand() % 500 + 100;
		if( current_needs > needs )
		{
			current_needs = needs;
		}
		int ch = chooseStoreHouse();
		StoreHouse *store = store_houses + ch;
		if( lockStore( store , 1 ) )
		{
			if( store->product_count >= current_needs )
			{
				store->product_count -= current_needs;
				store->use_count++;
				needs -= current_needs;
			}
			unlockStore( store );
		} else
		{
		}
		usleep( 1000 * ( 50 + ( rand() % 10 ) ) );
	}
	customers_unpleased -= 1;
	printf( "customer %i is done\n" , task->th_pid );
	fflush(stdout);
	return 0;
}
int loaderRoutine( Task *task )
{
	srand( clock() );
	while( customers_unpleased > 0 )
	{
		int ch = chooseStoreHouse();
		StoreHouse *store = store_houses + ch;
		if( lockStore( store , 1 ) )
		{
			//printf( "load \n" );fflush(stdout);
			store->product_count += rand() % 500 + 100;
			unlockStore( store );
		} else
		{
		}
		usleep( 1000 * 1 );
	}
	printf( "loader is done\n");
	return 0;
}
int main( int argc , char **argv )
{
	customers_unpleased = NUM_CUSTOMERS;
	int i;
	srand( clock() );
	for( i = 0; i < NUM_STORE_HOUSES; i++ )
	{
		store_houses[ i ].product_count = rand() % 100;
		store_houses[ i ].lock = 1;
		store_houses[ i ].use_count = 0;
	}
	for( i = 0; i < NUM_CUSTOMERS; i++ )
	{
		customers[ i ] = createTask( customerRoutine  , 0x100000 );
	}
	loader = createTask( loaderRoutine , 0x100000 );
    while( customers_unpleased > 0 )
    {
		usleep( 1000 * 100 );
    }
	for( i = 0; i < NUM_STORE_HOUSES; i++ )
	{
		printf( "store %i was used %i times \n" , i , store_houses[ i ].use_count );
	}
	for( i = 0; i < NUM_CUSTOMERS; i++ )
	{
		destroyTask( customers[ i ] );
	}
	destroyTask( loader );
    return 0;
}