#include "panel_factory.h"
#include "panel_interface.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

/*
* Factory constructor
*/
void panel_factory_init(panel_factory_t *const pf, void *opts, size_t opts_size)
{
    *pf = (panel_factory_t){
        .opts = arena_memdup(&pf->arena, opts, opts_size),
        .opts_size = opts_size,
    };
}

/*
* Create new panel via opts provided to the 'panel_factory_init'
*/
panel_t *panel_factory_create(panel_factory_t *const pf)
{
    panel_opts_t *base_opts = pf->opts;
    panel_t *new = base_opts->ifce.alloc(&pf->arena);
    panel_init(new, pf->opts);
    return new;
}

/*
* Beware! all object created with this factory will be destroyed
*/
void panel_factory_deinit(panel_factory_t *const pf)
{
    arena_free(&pf->arena);
}
