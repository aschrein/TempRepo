#ifndef TEXT_H
#define TEXT_H
#include <stdint.h>
#include <stdlib.h>
typedef struct Line_t
{
    char *chars;
    uint32_t max_length;
    uint32_t length;
    struct Line_t *next , *prev;
} Line;
static inline int strCmp( char const *a , char const *b ){ while( *a != '\0' && *a++ == *b++ ); return *a == *b && *b == '\0'; }
static inline void strcopy( char *dst , char const *src ){ while( *dst++ = *src++ ); }
typedef struct
{
    Line *lines_head;
    uint32_t lines_count;
} Text;
void insertCharacter( Line * , uint32_t , char );
inline static void addCharacter( Line *line , char c )
{
    return insertCharacter( line , line->length , c );
}
void mergeLineBack( Text * , uint32_t );
void divedeLineForward( Text * , uint32_t , uint32_t );
void insertCharacterInText( Text * , uint32_t , uint32_t , char );
char removeCharacter( Line* , uint32_t );
char removeCharacterInText( Text* , uint32_t , uint32_t );
Line *getLine( Text * , uint32_t );
Line *insertLine( Text * , uint32_t );
Line *removeLine( Text * , uint32_t );
inline static Line *addLine( Text *text )
{
    return insertLine( text , text->lines_count );
}
size_t getTextLength( Text * );
void bake( Text * , char * );
Text *createText( char const * );
Text *loadText( char const * );
void destroyText( Text * );
#endif
