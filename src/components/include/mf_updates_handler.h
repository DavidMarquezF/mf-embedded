#ifndef _MF_UPDATES_HANDLER_H_
#define _MF_UPDATES_HANDLER_H_
#include <stddef.h>
void download_update_coap(void); //TODO:  Temporary for tests, remove when done
void mf_updates_handler_init(void);
void mf_updates_handler_init_check_if_updated(void);

/**
 * Used to notify the cloud that the update has finished successfully or not
 */
void mf_updates_handler_cloud_login(void);
#endif