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
    memset( out , 0 , sizeof( EventSystem ) );
    out->text = text;
    return out;
}
void destroyEventSystem( EventSystem *system )
{
    free( system );
}
void applyEvent( EventSystem *system, uint32_t x , uint32_t y , uint32_t data , EventType type )
{
    if( POS( system->head + 1 ) == POS( system->tail ) )
    {
        system->tail = POS( system->tail + 1 );
    }
    Event *event = system->events + system->head;
    Event *inv = system->inverses + system->head;

    memset( event , 0 , sizeof( Event ) );
    memset( inv , 0 , sizeof( Event ) );
    event->type = type;
    event->cur_x = x;
    event->cur_y = y;
    event->data = data;

    system->head = POS( system->head + 1 );
    uint32_t cursor = system->head;
    while( cursor != system->tail )
    {
        Event *event = system->events + cursor;
        event->type = 0;
        cursor = POS( cursor + 1 );
    }
    fireEvent( system , event , inv );
}
void undo( EventSystem *system )
{
    if( system->head != system->tail )
    {
        system->head = POS( system->head - 1 );
        Event *event = system->inverses + system->head;
        fireEvent( system , event , NULL );
    }
}
void redo( EventSystem *system )
{
    if( POS( system->head + 1 ) != POS( system->tail ) )
    {
        uint32_t new_head = POS( system->head + 1 );
        Event *event = system->events + system->head;
        if( event->type )
        {
            system->head = new_head;
            fireEvent( system , event , NULL );
        }
    }
}
