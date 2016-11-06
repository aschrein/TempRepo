#include <ncurses.h>
#include <string.h>

#include "text.h"
#include "Event.h"
#include <fcntl.h>
void drawText( Text *text , uint32_t origin_x ,
    uint32_t origin_y , uint32_t width , uint32_t height ,
    uint32_t offsetx , uint32_t offsety )
{
    Line *line = getLine( text , offsety );
    uint32_t x = 0 , y = 0 , char_counter = offsetx;
    for( y = 0; y < height; y++ )
    {
        for( x = 0; x < width; x++ )
        {
            char c = ' ';
            if( line && char_counter < line->length )
            {
                c = line->chars[ char_counter++ ];
            }
            mvprintw( y + origin_y , x + origin_x , "%c" , c );
        }
        mvprintw( y + origin_y , origin_x - 4 , "   " );
        if( line )//&& char_counter == line->length )
        {
            mvprintw( y + origin_y , origin_x - 4 , "%i" , offsety );
            offsety++;
            char_counter = offsetx;
            line = line->next;
        }
    }
}
void drawFrame( uint32_t origin_x , uint32_t origin_y , uint32_t width , uint32_t height )
{
    uint32_t x = 0 , y = 0 , char_counter = 0;
    for( y = 1; y < height + 1; y++ )
    {
        mvprintw( y + origin_y , origin_x , "|" );
        mvprintw( y + origin_y , width + origin_x , "|" );
    }
    for( x = 0; x < width; x++ )
    {
        mvprintw( origin_y , x + origin_x , "_" );
        if( x > 0 )
        mvprintw( height + origin_y , x + origin_x , "_" );
    }
}
/*TODO
line dividing, undo redo , save open , file tree
*/
Text *text;
static EventSystem *es;
int offsety = 0 , offsetx = 0;
int cur_x = 0 , cur_y = 0;
int textEditor()
{
    int ch = getch();
    switch( ch )
    {
        case KEY_UP:
        {
            cur_y--;
        }
        break;
        case KEY_DOWN:
        {
            cur_y++;
        }
        break;
        case KEY_LEFT:
        {
            cur_x--;
        }
        break;
        case KEY_RIGHT:
        {
            cur_x++;
        }
        break;
        case KEY_BACKSPACE:
        {
            applyEvent( es , cur_x , cur_y , ch , BACK_SPACE );
            cur_x = es->cur_x;
            cur_y = es->cur_y;
        }
        break;
        case 10:
        {
            applyEvent( es , cur_x , cur_y , 0 , ENTER );
            cur_x = es->cur_x;
            cur_y = es->cur_y;
        }
        break;
        case 17://ctrl-q
        {
            return 1;
        }
        break;
        case 26://ctrl-z
        {
            undo( es );
            cur_x = es->cur_x;
            cur_y = es->cur_y;
        }
        break;
        case 18://ctrl-r
        {
            redo( es );
            cur_x = es->cur_x;
            cur_y = es->cur_y;
        }
        break;
    }
    if( ch > 31 && ch < 127 )
    {
        applyEvent( es , cur_x , cur_y , ch , INSERT_CHAR );
        cur_x = es->cur_x;
        cur_y = es->cur_y;
    }
    /*
    ctrl-z 26
    ctrl-r 18
    ctrl-s 19
    ctrl-q 17
    */
    //mvprintw( 0 , 0 , "       " );
    //mvprintw( 0 , 0 , "%c:%i" , ch , ch );
    int width , height;
    int panel_offsetx = 5 , panel_offsety = 4;
    getmaxyx( stdscr , height , width );
    height -= panel_offsety;
    width -= panel_offsetx;

    if( cur_x < 0 ){ cur_x = 0; }
    if( cur_x < offsetx ){ offsetx--; }
    if( offsetx < 0 ) offsetx = 0;
    if( cur_x >= width + offsetx ){ cur_x = width + offsetx; offsetx++; }

    if( cur_y < 0 ){ cur_y = 0; }
    if( cur_y < offsety ){ offsety = cur_y; }
    if( offsety < 0 ) offsety = 0;
    if( cur_y >= height + offsety ){ offsety++; }
    drawText( text , panel_offsetx , panel_offsety , width , height, offsetx , offsety );
    drawFrame( 4 , 3 , width  , height );
    drawFrame( 0 , 3 , 4 , height );
    wmove( stdscr , cur_y + panel_offsety - offsety , cur_x + panel_offsetx - offsetx );
    return 0;
}
int main()
{
    text = loadText( "test.txt" );
    if( text )
    {
        printf( "%i lines loaded\n" , text->lines_count );
    } else
    {
        printf( "file not loaded \n" );
        return 0;
    }
    es = createEventSystem( text );
    initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
    while( 1 )
    {
        if( textEditor() )
        {
            goto exit;
        }
        refresh();
    }
    exit:
    endwin();
    return 0;
}
