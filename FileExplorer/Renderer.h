#ifndef RENDERER_H
#define RENDERER_H
#include "text.h"
typedef int ( *UpdateFunc )( int width , int height );
void loop( UpdateFunc );
void drawText( Text *text , uint32_t origin_x ,
    uint32_t origin_y , uint32_t width , uint32_t height ,
    uint32_t offsetx , uint32_t offsety );
void drawBorder( uint32_t origin_x , uint32_t origin_y , uint32_t width , uint32_t height );
void drawMsg( char *text , uint32_t origin_x ,
    uint32_t origin_y , uint32_t width , uint32_t height ,
    uint32_t offsetx , uint32_t offsety );
#endif
