typedef struct event_base event_base;
struct event_base
{
    void (*free)(void * event);
};
