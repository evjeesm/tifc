#ifndef _EVENT_SYSTEM_H_
#define _EVENT_SYSTEM_H_

#include "hashmap.h"
#include "event_type.h"

typedef struct
{
    hashmap_t  *event_types; // <event_type_t>
    event_id_t next_event_id;
}
event_system_t;

typedef enum
{
    ES_OK = 0,
    ES_EVENT_ALREADY_EXISTS,
    ES_STATUS_NUM
}
es_status_t;

/*
* Initializer of the event system
*/
void es_init(event_system_t *const es);

/*
* Deinitializer.
*/
void es_deinit(event_system_t *const es);

/*
* Register a new event type in a system.
* Event type is described by an id.
**/
es_status_t es_create_publisher(event_system_t *const es, const event_type_opts_t *const event_type_opts);

/*
* Stop publishing
**/
void es_delete_publisher(event_system_t *const es, const event_id_t id);

/*
* Publisher interface for sending events.
**/
es_status_t es_send_event(event_system_t *const es, event_id_t id, void *const event_data);

/*
* 
**/
void es_subscribe(event_system_t *const es, const event_subscriber_opts_t *subsciber_opts);

/*
*  
**/
void es_unsubscribe(event_system_t *const es, const event_subscriber_opts_t *subsciber_opts);


#endif /*_EVENT_SYSTEM_H_*/
