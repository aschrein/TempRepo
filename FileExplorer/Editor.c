#include <ncurses.h>
#include <string.h>

#include "text.h"
#include "Event.h"
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
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
struct
{
    Text *text;
    char cur_filename[ PATH_MAX ];
    EventSystem *es;
    int offsety , offsetx;
    int cur_x , cur_y;
    int update;
} editor_state;
int textEditor()
{
    if( editor_state.update )
    {
        editor_state.update = 0;
        nodelay( stdscr , 1 );
    } else
    {
        nodelay( stdscr , 0 );
    }
    int ch = getch();
    switch( ch )
    {
        case KEY_UP:
        {
            editor_state.cur_y--;
        }
        break;
        case KEY_DOWN:
        {
            editor_state.cur_y++;
        }
        break;
        case KEY_LEFT:
        {
            editor_state.cur_x--;
        }
        break;
        case KEY_RIGHT:
        {
            editor_state.cur_x++;
        }
        break;
        case KEY_BACKSPACE:
        {
            applyEvent( editor_state.es , editor_state.cur_x , editor_state.cur_y , ch , BACK_SPACE );
            editor_state.cur_x = editor_state.es->cur_x;
            editor_state.cur_y = editor_state.es->cur_y;
        }
        break;
        case 10:
        {
            applyEvent( editor_state.es , editor_state.cur_x , editor_state.cur_y , 0 , ENTER );
            editor_state.cur_x = editor_state.es->cur_x;
            editor_state.cur_y = editor_state.es->cur_y;
        }
        break;
        case 17://ctrl-q
        {
            return 1;
        }
        break;
        case 26://ctrl-z
        {
            undo( editor_state.es );
            editor_state.cur_x = editor_state.es->cur_x;
            editor_state.cur_y = editor_state.es->cur_y;
        }
        break;
        case 18://ctrl-r
        {
            redo( editor_state.es );
            editor_state.cur_x = editor_state.es->cur_x;
            editor_state.cur_y = editor_state.es->cur_y;
        }
        break;
        case 19://ctrl-s
        {
            size_t chars_count = getTextLength( editor_state.text );
            char *chars = ( char * )malloc( chars_count );
            bake( editor_state.text , chars );
            int fd = open( editor_state.cur_filename , O_WRONLY | O_TRUNC );
            if( fd )
            {
                write( fd , chars , chars_count - 1 );
                close( fd );
            }
            free( chars );
        }
        break;
    }
    if( ch > 31 && ch < 127 )
    {
        applyEvent( editor_state.es , editor_state.cur_x , editor_state.cur_y , ch , INSERT_CHAR );
        editor_state.cur_x = editor_state.es->cur_x;
        editor_state.cur_y = editor_state.es->cur_y;
    }
    mvprintw( 0 , 0 , "ctrl-q to exit, ctrl-s to save changes, ctrl-z/r undo redo  " );
    mvprintw( 1 , 0 , "current file:%s" , editor_state.cur_filename );
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

    if( editor_state.cur_x < 0 ){ editor_state.cur_x = 0; }
    if( editor_state.cur_x < editor_state.offsetx ){ editor_state.offsetx--; }
    if( editor_state.offsetx < 0 ) editor_state.offsetx = 0;
    if( editor_state.cur_x >= width + editor_state.offsetx ){ editor_state.cur_x = width + editor_state.offsetx; editor_state.offsetx++; }

    if( editor_state.cur_y < 0 ){ editor_state.cur_y = 0; }
    if( editor_state.cur_y < editor_state.offsety ){ editor_state.offsety = editor_state.cur_y; }
    if( editor_state.cur_y > getLinesCount( editor_state.text ) ){ editor_state.cur_y = getLinesCount( editor_state.text ); }
    if( editor_state.offsety < 0 ) editor_state.offsety = 0;
    if( editor_state.cur_y >= height + editor_state.offsety ){ editor_state.offsety++; }
    drawText( editor_state.text , panel_offsetx , panel_offsety , width , height, editor_state.offsetx , editor_state.offsety );
    drawFrame( 4 , 3 , width  , height );
    drawFrame( 0 , 3 , 4 , height );
    wmove( stdscr , editor_state.cur_y + panel_offsety - editor_state.offsety , editor_state.cur_x + panel_offsetx - editor_state.offsetx );
    return 0;
}
WINDOW *mainwin;
void initWindow()
{
    //printf( "init()" );
    mainwin = initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
}
void releaseWindow()
{
    //noraw();
    //echo();
    //delwin( mainwin );
    endwin();
    //refresh();
    //printf( "release()" );
}
int main( int argc , char **argv )
{
    signal( SIGWINCH , SIG_IGN );
    memset( &editor_state , 0 , sizeof( editor_state ) );
    printf( "loading file %s\n" , argv[ 1 ] );
    editor_state.text = loadText( argv[ 1 ] );
    if( editor_state.text )
    {
        strcopy( editor_state.cur_filename , argv[ 1 ] );
        printf( "%i lines loaded\n" , getLinesCount( editor_state.text ) );
    } else
    {
        printf( "file not loaded \n" );
        return -1;
    }
    editor_state.es = createEventSystem( editor_state.text );
    initWindow();

    editor_state.update = 1;
    while( 1 )
    {
        //clear();
        int res = textEditor();
        if( res )
        {
            goto exit;
        }
        refresh();
    }
    exit:
    destroyText( editor_state.text );
    destroyEventSystem( editor_state.es );
    releaseWindow();
    return 0;
}
