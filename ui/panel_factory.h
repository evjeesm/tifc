#ifndef _PANEL_FACTORY_H_
#define _PANEL_FACTORY_H_

#include "panel.h"
#include "panel_interface.h"
#include "arena.h"

typedef struct panel_factory panel_factory_t;

struct panel_factory
{
    Arena arena;
    void *opts;
    size_t opts_size;
};


void panel_factory_init(panel_factory_t *const pf, void *opts, size_t opts_size);

panel_t *panel_factory_create(panel_factory_t *const pf);

void panel_factory_deinit(panel_factory_t *const pf);


#endif/*_PANEL_FACTORY_H_*/
