
#include <stdbool.h>
#include "mf_button.h"
#include "mf_hw_button.h"

static bool lastValue;
static oc_resource_t *resource;

static void
get_button(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  (void)user_data;
  oc_rep_start_root_object();
  switch (iface_mask) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
  /* fall through */
  case OC_IF_S:
    oc_rep_set_boolean(root, value, lastValue);
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
  
}

static void button_pressed(void){
  lastValue = !lastValue;
  if(resource)
    oc_notify_observers(resource);
}


uint8_t mf_button_create_resource(oc_resource_t *button_resource){
  resource = button_resource;
  oc_resource_bind_resource_type(button_resource, "oic.r.button");
  oc_resource_bind_resource_interface(button_resource, OC_IF_BASELINE);
  oc_resource_bind_resource_interface(button_resource, OC_IF_S);
  oc_resource_set_default_interface(button_resource, OC_IF_S);
    oc_resource_set_observable(button_resource, true);
  oc_resource_set_request_handler(button_resource, OC_GET, get_button, NULL);

  return 0;
}

uint8_t mf_button_init(void){
  lastValue = false;
  return mf_hw_button_init(button_pressed);
}

uint8_t mf_button_destroy(void){
  resource = NULL;
  return mf_hw_button_destroy();
}

