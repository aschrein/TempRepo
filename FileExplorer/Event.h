#ifndef EVENT_H
#define EVENT_H
#include "text.h"
typedef enum
{
    INSERT_CHAR , BACK_SPACE , ENTER
} EventType;
typedef struct
{
    uint32_t cur_x , cur_y , data;
    EventType type;
} Event;
typedef struct
{
    Event events[ 0x100 ];
    Event inverses[ 0x100 ];
    int32_t pos , length;
    Text *text;
    uint32_t cur_x , cur_y;
} EventSystem;
EventSystem *createEventSystem( Text * );
void destroyEventSystem( EventSystem * );
void applyEvent( EventSystem * , uint32_t , uint32_t , uint32_t , EventType );
void undo( EventSystem * );
void redo( EventSystem * );
#endif
