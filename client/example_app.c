#include "app.h"
#include "tifc.h"
#include "test_element.h"

typedef struct example_app
{
    Arena arena;
    const char *title;
}
example_app_t;


/* lets link client app staticly,
 * I will use global symbol for user data */
void *app_get_data(void)
{
    static example_app_t data;
    return &data;
}


void app_init(void *app, tifc_t *tifc)
{
    S_LOG(LOGGER_DEBUG, "example_app::app_init()\n");
    example_app_t *_app = app;
    UNUSED(tifc);

    _app->title = "Hello Tifc App!\n";
    _app->arena = (Arena){ 0 };

    /* here goes ui setup code */
    test_element_opts_t *opts = & (test_element_opts_t) {
        .impl = test_element_get_impl(),
        .arena =  &_app->arena,
    };

    
    // ui_create_element(&tifc->ui, 
    // tifc->ui
}


void app_deinit(void *app, tifc_t *tifc)
{
    S_LOG(LOGGER_DEBUG, "example_app::app_deinit()\n");
    UNUSED(app, tifc);
}
