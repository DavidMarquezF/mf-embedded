#ifndef _MF_OC_HELPERS_H_
#define _MF_OC_HELPERS_H_
#include <stdbool.h>
#include "oc_api.h"
/**
* helper function to check if the POST input document contains 
* the common readOnly properties or the resouce readOnly properties
* @param name the name of the property
* @return the error_status, e.g. if error_status is true, then the input document contains something illegal
*/
bool check_on_readonly_common_resource_properties(oc_string_t name, bool error_state);
#endif