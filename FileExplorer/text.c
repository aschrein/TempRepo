#include "text.h"
#include <fcntl.h>
static void moveArray( Line *line , size_t new_size )
{
    char *new_chars  =( char * )malloc( new_size );
    if( line->chars )
    {
        memcpy( new_chars , line->chars , line->length );
        free( line->chars );
    }
    line->max_length = new_size;
    line->chars = new_chars;
}
void insertCharacter( Line *line , uint32_t pos , char ch )
{
    if( pos < line->length )
    {
        if( line->length == line->max_length )
        {
            moveArray( line , line->length + 0x40 );
        }
        int i = line->length;
        for(; i > pos; i-- )
        {
            line->chars[ i ] = line->chars[ i - 1 ];
        }
        line->chars[ pos ] = ch;
        line->length++;
    } else
    {
        moveArray( line , pos + 0x10 );
        int i = line->length;
        for(; i < pos; i++ )
        {
            line->chars[ i ] = ' ';
        }
        line->chars[ pos ] = ch;
        line->length = pos + 1;
    }
}
char removeCharacter( Line *line , uint32_t pos )
{
    if( pos >= line->length )
    {
        return;
    }
    char c = line->chars[ pos ];
    int i = pos;
    for(; i < line->length - 1; i++ )
    {
        line->chars[ i ] = line->chars[ i + 1 ];
    }
    line->length--;
    if( line->length < line->max_length - 0x60 )
    {
       moveArray( line , line->length + 0x10 );
    }
    return c;
}
Line *getLine( Text *text , uint32_t pos )
{
    if( !text )
    {
        return NULL;
    }
    Line *line = text->lines_head;
    while( pos && line )
    {
        line = line->next;
        pos--;
    }
    return line;
}
static Line *createLine()
{
    Line *new_line = ( Line* )malloc( sizeof( Line ) );
    new_line->chars = NULL;
    new_line->max_length = 0;
    new_line->length = 0;
    new_line->next = NULL;
    new_line->prev = NULL;
    return new_line;
}
static void destroyLine( Line *line )
{
    if( line->chars )
    {
        free( line->chars );
    }
    free( line );
}
Line *removeLine( Text *text , uint32_t pos )
{
    if( pos < text->lines_count )
    {
        Line *line = getLine( text , pos );
        if( line->next )
        {
            line->next->prev = line->prev;
        }
        if( line->prev )
        {
            line->prev->next = line->next;
        } else
        {
            text->lines_head = line;
        }
        destroyLine( line );
    }
}
Line *insertLine( Text *text , uint32_t pos )
{
    Line *new_line = createLine();
    if( pos < text->lines_count )
    {
        Line *line = getLine( text , pos );
        if( line->prev )
        {
            line->prev->next = new_line;
            new_line->prev = line->prev;
            new_line->next = line;
            line->prev = new_line;
        } else
        {
            new_line->next = text->lines_head;
            text->lines_head = new_line;
        }
        text->lines_count++;
    } else
    {
        if( pos == 0 )
        {
            text->lines_head = new_line;
        } else
        {
            int i = text->lines_count;
            Line *tail = getLine( text , text->lines_count - 1 );
            if( !tail )
            {
                tail = createLine();
                text->lines_head = tail;
                i++;
            }
            for(; i < pos + 1; i++ )
            {
                Line *new_line = createLine();
                tail->next = new_line;
                new_line->prev = tail;
                tail = new_line;
            }
        }
        text->lines_count = pos + 1;
    }
}
size_t getTextLength( Text *text )
{
    Line *line = text->lines_head;
    size_t size = 0;
    while( line )
    {
        size += line->length + 1;
        line = line->next;
    }
    return size + 1;
}
static void bakeLine( Line *line , char *chars )
{
    memcpy( chars , line->chars , line->length );
}
void bake( Text *text , char *chars )
{
    Line *line = text->lines_head;
    while( line )
    {
        bakeLine( line , chars );
        chars += line->length;
        *chars++ = '\n';

        line = line->next;
    }
    *chars++ = '\0';
}
Text *createText( char const *chars )
{
    Line *cur_line = createLine();
    Text *out = ( Text* )malloc( sizeof( Text ) );
    out->lines_head = cur_line;
    while( *chars )
    {
        char c = *chars++;
        if( c == '\n' )
        {
            Line *new_line = createLine();
            cur_line->next = new_line;
            new_line->prev = cur_line;
            cur_line = new_line;
        } else
        {
            addCharacter( cur_line , c );
        }
    }
    return out;
}
void insertCharacterInText( Text *text , uint32_t x , uint32_t y , char c )
{
    Line *line = getLine( text , y );
    if( line )
    {
        insertCharacter( line , x , c );
    } else
    {
        line = insertLine( text , y );
        insertCharacter( line , x , c );
    }
}
void mergeLineBack( Text *text , uint32_t y )
{
    Line *line = getLine( text , y );
    if( line && line->prev )
    {
        Line *new_line = createLine();
        new_line->chars = ( char * )malloc( line->length + line->prev->length );
        new_line->length = new_line->max_length = line->prev->length + line->length;

        memcpy( new_line->chars , line->prev->chars , line->prev->length );
        memcpy( new_line->chars + line->prev->length , line->chars , line->length );
        if( line->prev->prev )
        {
            line->prev->prev->next = new_line;
            new_line->prev = line->prev->prev;
        } else
        {
            text->lines_head = new_line;
        }
        if( line->next )
        {
            line->next->prev = new_line;
            new_line->next = line->next;
        }
        destroyLine( line );
        destroyLine( line->prev );
        text->lines_count--;
    }
}
void divedeLineForward( Text *text , uint32_t x , uint32_t y )
{
    Line *line = getLine( text , y );
    if( line )
    {
        Line *new_line = createLine();
        new_line->chars = ( char * )malloc( line->length - x );
        new_line->length = new_line->max_length = line->length - x;
        memcpy( new_line->chars , line->chars + x , new_line->length );
        line->length = x;
        if( line->next )
        {
            line->next->prev = new_line;
            new_line->next = line->next;
        }
        line->next = new_line;
        new_line->prev = line;
        text->lines_count++;
    }
}
char removeCharacterInText( Text *text , uint32_t x , uint32_t y )
{
    Line *line = getLine( text , y );
    if( line )
    {
        return removeCharacter( line , x );
    }
    return '\0';
}
Text *loadText( char const *filename )
{
    int fd = open( filename , O_RDONLY );
    Text *out = NULL;
    if( fd )
    {
        //printf( "file %s opened!\n" , filename );
        Line *cur_line = createLine();
        out = ( Text* )malloc( sizeof( Text ) );
        out->lines_head = cur_line;
        out->lines_count = 1;
        char buf[ 0x100 + 1 ];
        int buf_len = 0 , i = 0;
        while( ( buf_len = read( fd , buf , 0x100 ) ) > 0 )
        {
            buf[ 0x100 ] = '\0';
            //printf( "read:%s" , buf );
            for( i = 0; i < buf_len; i++ )
            {
                char c = buf[ i ];
                if( c == '\r' )
                {
                    continue;
                }
                if( c == '\n' )
                {
                    Line *new_line = createLine();
                    cur_line->next = new_line;
                    new_line->prev = cur_line;
                    cur_line = new_line;
                    out->lines_count++;
                } else
                {
                    /*if( c == '\t' )
                    {
                        addCharacter( cur_line , ' ' );
                        addCharacter( cur_line , ' ' );
                        addCharacter( cur_line , ' ' );
                        addCharacter( cur_line , ' ' );
                    } else*/
                    {
                        addCharacter( cur_line , c );
                    }
                }
            }
        }
        close( fd );
    }
    return out;
}
void destroyText( Text *text )
{
    Line *line = text->lines_head;
    while( line )
    {
        Line *next_line = line->next;
        destroyLine( line );
        line = next_line;
    }
    free( text );
}
