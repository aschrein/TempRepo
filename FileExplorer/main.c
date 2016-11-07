#include <ncurses.h>
#include <string.h>

#include "text.h"
#include "Event.h"
#include <fcntl.h>
#include <dirent.h>
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
            int fd = open( editor_state.cur_filename , O_WRONLY );
            write( fd , chars , chars_count );
            close( fd );
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
    mvprintw( 0 , 0 , "ctrl-q to exit to directory view, ctrl-s to save changes, ctrl-z/r undo redo  " );
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
    if( editor_state.cur_y >= editor_state.text->lines_count ){ editor_state.cur_y = editor_state.text->lines_count - 1; }
    if( editor_state.offsety < 0 ) editor_state.offsety = 0;
    if( editor_state.cur_y >= height + editor_state.offsety ){ editor_state.offsety++; }
    drawText( editor_state.text , panel_offsetx , panel_offsety , width , height, editor_state.offsetx , editor_state.offsety );
    drawFrame( 4 , 3 , width  , height );
    drawFrame( 0 , 3 , 4 , height );
    wmove( stdscr , editor_state.cur_y + panel_offsety - editor_state.offsety , editor_state.cur_x + panel_offsetx - editor_state.offsetx );
    return 0;
}
struct
{
    char cur_directory[ PATH_MAX ];
    char temp_buffer[ PATH_MAX ];
    int cur_file;
    int max_file;
    int offsety;
    int update;
} dir_state;
int drawDirectory()
{
    int open_dir = 0;
    if( dir_state.update )
    {
        dir_state.update = 0;
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
            dir_state.cur_file--;
        }
        break;
        case KEY_DOWN:
        {
            dir_state.cur_file++;
        }
        break;
        case 10:
        {
            open_dir = 1;
        }
        break;
        case 17:
        {
            return 1;
        }
        break;
    }
    int width , height;
    int panel_offsetx = 5 , panel_offsety = 4;
    getmaxyx( stdscr , height , width );

    height -= panel_offsety;
    width -= panel_offsetx;

    if( dir_state.cur_file < 0 ){ dir_state.cur_file = 0; }
    if( dir_state.cur_file < dir_state.offsety ){ dir_state.offsety--; }
    if( dir_state.offsety < 0 ) dir_state.offsety = 0;
    if( dir_state.cur_file >= height + dir_state.offsety ){ dir_state.cur_file = height + dir_state.offsety; dir_state.offsety++; }
    if( dir_state.cur_file >= dir_state.max_file ) dir_state.cur_file = dir_state.max_file - 1;

    drawFrame( panel_offsetx - 1 , panel_offsety - 1 , width  , height );
    drawFrame( 0 , panel_offsety - 1 , 4 , height );

    mvprintw( 0 , 0 , "ctrl-q to quit, enter to open file                                          " );

    DIR *d;
	struct dirent *dir;
	d = opendir( dir_state.cur_directory );
    int y = -dir_state.offsety;
    dir_state.max_file = 0;

	if( d )
	{
		while( dir = readdir( d ) )
		{
			if( dir->d_type == DT_DIR && dir_state.max_file == dir_state.cur_file && open_dir )
			{
                strcopy( dir_state.temp_buffer , dir_state.cur_directory );
                //strcopy( temp_buffer + 2 , dir->d_name );
                int ptr = strlen( dir_state.cur_directory );
                dir_state.temp_buffer[ ptr ] = '/';
                strcopy( dir_state.temp_buffer + ptr + 1 , dir->d_name );
                ptr += strlen( dir_state.temp_buffer );

                //mvprintw( 0 , 0 , temp_buffer );
                realpath( dir_state.temp_buffer , dir_state.cur_directory );
                dir_state.cur_file = 0;
                dir_state.offsety = 0;
                //strcopy( cur_directory , temp_buffer );//
                dir_state.update = 1;
			} else if( dir->d_type == DT_REG && dir_state.max_file == dir_state.cur_file && open_dir )
            {
                strcopy( dir_state.temp_buffer , dir_state.cur_directory );
                int ptr = strlen( dir_state.cur_directory );
                dir_state.temp_buffer[ ptr ] = '/';
                strcopy( dir_state.temp_buffer + ptr + 1 , dir->d_name );
                ptr += strlen( dir_state.temp_buffer );
                realpath( dir_state.temp_buffer , editor_state.cur_filename );
                return -1;
            }
            if( y >= 0 )
            {
                if( dir->d_type == DT_DIR )
                {
                    mvprintw( panel_offsety + y , panel_offsetx , "%s/" , dir->d_name );
                } else
                {
                    mvprintw( panel_offsety + y , panel_offsetx , "%s*" , dir->d_name );
                }

                mvprintw( panel_offsety + y , 1 , "   " );
                mvprintw( panel_offsety + y , 1 , "%i" , dir_state.max_file );
                int len = strlen( dir->d_name );
                int i;
                for( i = len + 1; i < width - 1; i++ )
                {
                    mvprintw( panel_offsety + y , panel_offsetx + i , " " );
                }
            }
            dir_state.max_file++;
            y++;
		}
		closedir( dir );
	}
    while( y < height - 1 )
    {
        if( y > 0 )
        {
            int i;
            for( i = 0; i < width - 1; i++ )
            {
                mvprintw( panel_offsety + y , panel_offsetx + i , " " );
            }
            mvprintw( panel_offsety + y , 1 , "   " );
        }
        y++;
    }
    {
        int i;
        getmaxyx( stdscr , height , width );
        for( i = 0; i < width; i++ )
        {
            mvprintw( 1 , i , " " );
        }
        mvprintw( 1 , 0 , "current directory:%s" , dir_state.cur_directory );
    }
    wmove( stdscr , panel_offsety + dir_state.cur_file - dir_state.offsety , panel_offsetx );

    return 0;
}
int main()
{
    initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
    memset( &editor_state , 0 , sizeof( editor_state ) );
    memset( &dir_state , 0 , sizeof( dir_state ) );
    dir_state.cur_directory[ 0 ] = '.';
    dir_state.cur_directory[ 1 ] = '/';
    dir_state.cur_directory[ 2 ] = '\0';
    dir_state.update = 1;
    editor_state.update = 1;
    int mode = 0;
    while( 1 )
    {
        if( mode == 0 )
        {
            if( editor_state.text )
            {
                destroyText( editor_state.text );
                destroyEventSystem( editor_state.es );
                editor_state.text = NULL;
            }
            int res = drawDirectory();
            if( res == 1 )
            {
                goto exit;
            } else if( res == -1 )
            {
                mode = 1;
                dir_state.update = 1;
            }
        } else if( mode == 1 )
        {
            if( !editor_state.text )
            {
                editor_state.text = loadText( editor_state.cur_filename );
                if( editor_state.text )
                {
                    //printf( "%i lines loaded\n" , editor_state.text->lines_count );
                } else
                {
                    //printf( "file not loaded \n" );
                    mode = 0;
                }
                editor_state.es = createEventSystem( editor_state.text );
            }
            int res = textEditor();
            if( res )
            {
                mode = 0;
                editor_state.update = 1;
                editor_state.offsetx = 0;
                editor_state.offsety = 0;
                editor_state.cur_x = 0;
                editor_state.cur_y = 0;
            }
        }
        refresh();
    }
    exit:
    endwin();
    return 0;
}
