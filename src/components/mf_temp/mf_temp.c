
#include <stdbool.h>
#include "mf_temp.h"
#include "mf_hw_temp.h"

static double temp = 5.0,
              min_C = 0.0, max_C = 100.0, min_K = 273.15, max_K = 373.15,
              min_F = 32, max_F = 212;
typedef enum { C = 100, F, K } units_t;
units_t temp_units = C;


static void
get_temp(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  (void)user_data;
  PRINT("GET_temp:\n");
  bool invalid_query = false;
  char *units;

  float val;
  mf_hw_temp_get_value(&val);

  //TODO: return error if get fails
  temp = val;

  units_t u = temp_units;
  int units_len = oc_get_query_value(request, "units", &units);
  if (units_len != -1) {
    if (units[0] == 'K') {
      u = K;
    } else if (units[0] == 'F') {
      u = F;
    } else if (units[0] != 'C') {
      u = C;
    } else {
      invalid_query = true;
    }
  }

  oc_rep_start_root_object();
  switch (iface_mask) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
  /* fall through */
  case OC_IF_S:
    switch (u) {
    case C:
      oc_rep_set_text_string(root, units, "C");
      oc_rep_set_double(root, temperature, temp);
      break;
    case F:
      oc_rep_set_text_string(root, units, "F");
      oc_rep_set_double(root, temperature, temp * 9.0 / 5 + 32);
      break;
    case K:
      oc_rep_set_text_string(root, units, "K");
      oc_rep_set_double(root, temperature, temp + 273.15);
      break;
    }
    break;
  default:
    break;
  }

  oc_rep_set_array(root, range);
  switch (u) {
  case C:
    oc_rep_add_double(range, min_C);
    oc_rep_add_double(range, max_C);
    break;
  case K:
    oc_rep_add_double(range, min_K);
    oc_rep_add_double(range, max_K);
    break;
  case F:
    oc_rep_add_double(range, min_F);
    oc_rep_add_double(range, max_F);
    break;
  }
  oc_rep_close_array(root, range);

  oc_rep_end_root_object();

  if (invalid_query)
    oc_send_response(request, OC_STATUS_FORBIDDEN);
  else
    oc_send_response(request, OC_STATUS_OK);
}



uint8_t mf_temp_create_resource(oc_resource_t *temp_resource){
  oc_resource_bind_resource_type(temp_resource, "oic.r.temperature");
  oc_resource_bind_resource_interface(temp_resource, OC_IF_S);
  oc_resource_set_default_interface(temp_resource, OC_IF_S);
  oc_resource_set_discoverable(temp_resource, true);
  oc_resource_set_periodic_observable(temp_resource, 1);
  oc_resource_set_request_handler(temp_resource, OC_GET, get_temp, NULL);
  oc_resource_tag_func_desc(temp_resource, OC_ENUM_HEATING);
  oc_resource_tag_pos_desc(temp_resource, OC_POS_CENTRE);

  return 0;
}

uint8_t mf_temp_init(void){
  return mf_hw_temp_init();
}

uint8_t mf_temp_destroy(void){
  return mf_hw_temp_destroy();
}

