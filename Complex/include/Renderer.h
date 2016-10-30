#ifndef RENDERER_H
#define RENDERER_H
typedef enum
{
	CONTINUE , BREAK
} Flow;
void *createWindow( int x , int y , int width , int height , Flow ( *update )( void * , void * ) , void * );
void destroyWindow( void * );
typedef struct
{
	int x , y;
} ivec2;
ivec2 getDeltaMouse( void * );
typedef enum
{
	NONE , PRESS , DOWN , UP
} PressMode;
void fillRect( void * , int x , int y , char const *text );
PressMode drawButton( void * , int x , int y , int width , int height );
void drawText( void * , int x , int y , char const *text );
ivec2 getTextSize( void * , char const *text );
void drawLine( void * , int x0 , int y0 , int x1 , int y1 );
#endif
