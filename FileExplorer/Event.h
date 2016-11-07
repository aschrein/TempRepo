#ifndef EVENT_H
#define EVENT_H
#include "text.h"
typedef enum
{
    INSERT_CHAR = 1 , BACK_SPACE = 2 , ENTER = 3
} EventType;
typedef struct
{
    uint32_t cur_x , cur_y , data;
    EventType type;
} Event;
#define MAX_EVENTS 0x100
#define POS( x ) ( ( x ) % MAX_EVENTS )
typedef struct
{
    //events ring buffer
    Event events[ MAX_EVENTS ];
    Event inverses[ MAX_EVENTS ];
    int32_t head , tail;
    Text *text;
    uint32_t cur_x , cur_y;
} EventSystem;
EventSystem *createEventSystem( Text * );
void destroyEventSystem( EventSystem * );
void applyEvent( EventSystem * , uint32_t x , uint32_t y , uint32_t character , EventType );
void undo( EventSystem * );
void redo( EventSystem * );
#endif
