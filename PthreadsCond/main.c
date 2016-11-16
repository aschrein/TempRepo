#include <unistd.h>
#include <pthread.h>
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
void counterRoutine()
{
	while( 1 )
	{
		pthread_mutex_lock( &mutex );
		count++;
		pthread_cond_signal( &cond );
		printf( "%i\n" , count );
		pthread_mutex_unlock( &mutex );
	}
}
void reseterRoutine()
{
	while( 1 )
	{
		pthread_mutex_lock( &mutex );
		while( count < 100 )
		{
			pthread_cond_wait( &cond , &mutex );
		}
		count = 0;
		pthread_mutex_unlock( &mutex );
	}
}
int main( int argc , char **argv )
{
	pthread_t counter , reseter;
	pthread_create( &counter , NULL , counterRoutine , NULL );
	pthread_create( &reseter , NULL , reseterRoutine , NULL );
	pthread_join( reseter , NULL );
	pthread_join( counter , NULL );
	pthread_cond_destroy( &cond );
	pthread_mutex_destroy( &mutex );
	return 0;
}