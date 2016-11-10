#include <ncurses.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include "text.h"
#include <signal.h>
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
WINDOW *mainwin;
void initWindow()
{
    //printf( "init()" );
    mainwin = initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
    refresh();
}
void releaseWindow()
{
    noraw();
    echo();
    //delwin( mainwin );
    endwin();
    //
    //printf( "release()" );
}
struct
{
    char cur_directory[ PATH_MAX ];
    char temp_buffer[ PATH_MAX ];
    char cur_filename[ PATH_MAX ];
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
                realpath( dir_state.temp_buffer , dir_state.cur_filename );
                releaseWindow();
                int chpid = fork();
                signal( SIGWINCH , SIG_IGN );
                if( chpid > 0 )
                {

                    int status;
                    //do
                    {
                        waitpid( chpid , &status , 0 );
                    }// while( WIFEXITED( status ) || WIFSIGNALED( status ) );
                    //printf( "childe exited\n" );
                    dir_state.update = 1;
                    initWindow();
                    return 0;
                } else if( chpid == 0 )
                {
                    char const **argv[ 3 ] =
                    {
                        "./Editor.out" ,
                        dir_state.cur_filename ,
                        NULL
                    };
                    execv( "./Editor.out" , argv );
                }
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

    initWindow();
    memset( &dir_state , 0 , sizeof( dir_state ) );
    dir_state.cur_directory[ 0 ] = '.';
    dir_state.cur_directory[ 1 ] = '/';
    dir_state.cur_directory[ 2 ] = '\0';
    dir_state.update = 1;
    while( 1 )
    {
        //clear();
        //waitpid( -1 , NULL , WNOHANG );
        int res = drawDirectory();
        if( res == 1 )
        {
            goto exit;
        }
        refresh();
    }
    exit:
    releaseWindow();
    return 0;
}
