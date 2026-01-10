#ifndef _EVENT_TYPE_H_
#define _EVENT_TYPE_H_

#include "stddef.h"
#include "sparse.h"
#include "arena.h"

typedef size_t event_id_t;
typedef struct event_type event_type_t;

typedef struct
{
    void (*subscribe) (void *const subscriber, struct event_type *const event_type);
    void (*unsubscribe) (void *const subscriber, struct event_type *const event_type);
    void (*dispatch) (void *const event_data, struct event_type *const event_type);
    void (*close) (struct event_type *const event_type);
}
event_publisher_interface_t;

typedef struct
{
    event_id_t type_id;
    event_publisher_interface_t impl;
    void *data;
}
event_type_opts_t;

struct event_type
{
    event_publisher_interface_t impl;
    void *data;
    Arena arena;
};

typedef struct
{
    void (*handle_event) (void *const subscriber, void *const event_data, void *const event_type_data);
    void (*handle_close) (void *const subscriber, void *const event_type_data);
}
event_subcriber_interface_t;

typedef struct
{
    event_subcriber_interface_t impl;
    void *subscriber_data;
}
event_subscriber_opts_t;


#endif/*_EVENT_TYPE_H_*/
