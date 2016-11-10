#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gfp.h>

static struct class *char_class = NULL;
static struct device *char_device = NULL;
static int mnumber = -1;
static struct page *allocated_pages = NULL;

static int dev_open( struct inode* , struct file* );
static int dev_release( struct inode* , struct file* );
static ssize_t dev_read( struct file* , char * , size_t , loff_t * );
static ssize_t dev_write( struct file* , char const * , size_t , loff_t * );
static struct file_operations fops =
{
	.open = dev_open ,
	.read = dev_read ,
	.write = dev_write ,
	.release = dev_release
};
static int dev_open( struct inode* n , struct file* f )
{
	return 0;
}
static int dev_release( struct inode* n , struct file* f )
{
	return 0;
}
//static char const * const msg = "hello from device!";
static ssize_t dev_read( struct file* fileep , char * data_ptr , size_t size , loff_t *offset )
{
	char *cast = ( char * )allocated_pages[ 0 ].virtual;
	int i = 0;
	for( int i = 0; i < size; i++ )
	{
		cast[ 0 ] = data_ptr[ i ];
	}
	return 0;
}
static ssize_t dev_write( struct file* fileep , char const * data_ptr , size_t size , loff_t *offset )
{
	char *cast = ( char * )allocated_pages[ 0 ].virtual;
	int i;
	for( int i = 0; i < size; i++ )
	{
		cast[ 0 ] = data_ptr[ i ];
	}
	return 0;
}
int init_module(void)
{
	allocated_pages = alloc_pages( GFP_KERNEL , 3 );
	mnumber = register_chrdev( 0 , "MyDevice" , &fops );
	char_class = class_create( mnumber , "MyClass" );
	char_device = device_create( char_class , NULL , MKDEV( mnumber , 0 ) , NULL , "MyDevice" );
	
	printk( "<1>hello world 1.\n" );
	return 0;
}
void cleanup_module(void)
{
	if( allocated_pages != NULL )
	{
		__free_pages( allocated_pages , 3 );
	}
	device_destroy( char_class , MKDEV( mnumber , 0 ) );
	class_unregister( char_class );
	class_destroy( char_class );
	unregister_chrdev( mnumber , "MyDevice" );
	printk( KERN_ALERT "module cleaned up\n" );
}
MODULE_LICENSE( "GPL" );
MODULE_VERSION( "0.1" );
