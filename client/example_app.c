#include "app.h"
#include "tifc.h"

typedef struct example_app
{
    const char *title;
}
example_app_t;


/* lets link client app statically,
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

    /* here goes ui setup code */
}


void app_deinit(void *app, tifc_t *tifc)
{
    S_LOG(LOGGER_DEBUG, "example_app::app_deinit()\n");
    UNUSED(app, tifc);
}
