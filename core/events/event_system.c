#include "event_system.h"
#include "assert.h"
#include "utils.h"

void es_init(event_system_t *const es)
{
    assert(es);

    *es = (event_system_t){0};

    es->event_types = hm_create(.key_size=sizeof(event_id_t),
                                .value_size=sizeof(event_type_t),
                                .hashfunc=hash_longlong);

    assert((es->event_types) && "failed to allocate hashmap");
}

void es_deinit(event_system_t *const es)
{
    assert(es);
    assert(es->event_types);

    hm_destroy(es->event_types);
}

es_status_t es_create_publisher(event_system_t *const es, const event_type_opts_t *const event_type_opts)
{
    event_type_t *event;

    hm_status_t status = hm_reserve(&es->event_types, &event_type_opts->type_id, (void**)&event);

    if (HM_ALREADY_EXISTS != status) return ES_EVENT_ALREADY_EXISTS;

    *event = (event_type_t)
    {
        .impl = event_type_opts->impl,
        .data = event_type_opts->data,
    };

    return ES_OK;
}


