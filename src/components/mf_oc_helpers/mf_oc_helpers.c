
#include "mf_oc_helpers.h"
#include "mf_log.h"

/**
* helper function to check if the POST input document contains 
* the common readOnly properties or the resouce readOnly properties
* @param name the name of the property
* @return the error_status, e.g. if error_status is true, then the input document contains something illegal
*/
bool check_on_readonly_common_resource_properties(oc_string_t name, bool error_state)
{
  if (strcmp(oc_string(name), "n") == 0)
  {
    error_state = true;
    PRINT("   property \"n\" is ReadOnly \n");
  }
  else if (strcmp(oc_string(name), "if") == 0)
  {
    error_state = true;
    PRINT("   property \"if\" is ReadOnly \n");
  }
  else if (strcmp(oc_string(name), "rt") == 0)
  {
    error_state = true;
    PRINT("   property \"rt\" is ReadOnly \n");
  }
  else if (strcmp(oc_string(name), "id") == 0)
  {
    error_state = true;
    PRINT("   property \"id\" is ReadOnly \n");
  }
  else if (strcmp(oc_string(name), "id") == 0)
  {
    error_state = true;
    PRINT("   property \"id\" is ReadOnly \n");
  }
  return error_state;
}
