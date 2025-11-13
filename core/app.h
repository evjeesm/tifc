#ifndef _APP_H_
#define _APP_H_

#include "tifc.h"

/*
* Interface for the client application code
*/

/*
* Prepare application before running.
*/
void app_init(void *app, tifc_t *tifc);

/*
* Free application resources.
*/
void app_deinit(void *app, tifc_t *tifc);

/*
* Returns an unallocated pointer to a user data.
*/
void *app_get_data(void);

#endif/* _APP_H_ */
