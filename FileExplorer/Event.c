#include "Event.h"
static void fireEvent( EventSystem *system , Event *event , Event *inv )
{
    switch( event->type )
    {
        case INSERT_CHAR:
        {
            insertCharacterInText( system->text , event->cur_x , event->cur_y , event->data );
            system->cur_x = event->cur_x + 1;
            system->cur_y = event->cur_y;
            if( inv )
            {
                inv->type = BACK_SPACE;
                inv->cur_x = event->cur_x + 1;
                inv->cur_y = event->cur_y;
            }
        }
        break;
        case BACK_SPACE:
        {
            if( event->cur_x == 0 && event->cur_y > 0 )
            {
                system->cur_x = getLine( system->text , event->cur_y - 1 )->length;
                system->cur_y = event->cur_y - 1;
                mergeLineBack( system->text , event->cur_y );
                if( inv )
                {
                    inv->type = ENTER;
                    inv->cur_x = system->cur_x;
                    inv->cur_y = system->cur_y;
                }
            } else
            {
                system->cur_x = event->cur_x - 1;
                system->cur_y = event->cur_y;
                char c = removeCharacterInText( system->text , event->cur_x - 1 , event->cur_y );
                if( inv )
                {
                    inv->type = INSERT_CHAR;
                    inv->data = c;
                    inv->cur_x = system->cur_x;
                    inv->cur_y = system->cur_y;
                }
            }
        }
        break;
        case ENTER:
        {
            divedeLineForward( system->text , event->cur_x , event->cur_y );
            system->cur_x = 0;
            system->cur_y = event->cur_y + 1;
            if( inv )
            {
                inv->type = BACK_SPACE;
                inv->cur_x = system->cur_x;
                inv->cur_y = system->cur_y;
            }
        }
        break;
    }
}
EventSystem *createEventSystem( Text *text )
{
    EventSystem *out = ( EventSystem* )malloc( sizeof( EventSystem ) );
    out->pos = -1;
    out->length = 0;
    out->text = text;
    out->cur_x = 0;
    out->cur_y = 0;
    return out;
}
//void destroyEventSystem( EventSystem * );
void applyEvent( EventSystem *system, uint32_t x , uint32_t y , uint32_t data , EventType type )
{
    if( system->pos == 0x100 - 1 && system->length )
    {
        int i;
        for( i = 0; i < system->length - 1; i++ )
        {
            system->events[ i ] = system->events[ i + 1 ];
        }
        system->pos--;
        system->length--;
    }
    system->pos++;
    system->length++;
    Event *event = system->events + system->pos;
    Event *inv = system->inverses + system->pos;
    memset( event , 0 , sizeof( Event ) );
    memset( inv , 0 , sizeof( Event ) );
    event->type = type;
    event->cur_x = x;
    event->cur_y = y;
    event->data = data;
    fireEvent( system , event , inv );
}
void undo( EventSystem *system )
{
    if( system->pos + 1 )
    {
        Event *event = system->inverses + system->pos;
        fireEvent( system , event , NULL );
        system->pos--;
    }
}
void redo( EventSystem *system )
{
    if( system->pos < system->length - 1 )
    {
        system->pos++;
        Event *event = system->events + system->pos;
        fireEvent( system , event , NULL );
    }
}
