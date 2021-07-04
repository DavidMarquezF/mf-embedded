
#include <stdbool.h>
#include "mf_semaphore.h"
#include "mf_hw_semaphore.h"


typedef struct {
  bool green;
  bool yellow;
  bool red; 
} semaphore_t;

static semaphore_t semaphore;

static void
get_semaphore(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  semaphore_t *semaphore = (semaphore_t *)user_data;
  PRINT("%d %d %d", semaphore->green, semaphore->yellow, semaphore->red);

  oc_rep_start_root_object();
  switch (iface_mask) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
  /* fall through */
  case OC_IF_A:
      oc_rep_set_boolean(root, green, semaphore->green);
      oc_rep_set_boolean(root, yellow, semaphore->yellow);
      oc_rep_set_boolean(root, red, semaphore->red);
  default:
    break;
  }
  oc_rep_end_root_object();

  oc_send_response(request, OC_STATUS_OK);
}

/**
 * @return: 0 if successful, 1 if bad request and 2 if it is not the key  
 */
static uint8_t set_semaphore_value_from_resp(oc_request_t *request,  oc_rep_t *rep, char * property_name, char * key, bool* light){  
  if (key && !strcmp(key, property_name)) {
    switch (rep->type) {
    case OC_REP_BOOL:
      *light = rep->value.boolean;
      return 0;
    default:
      oc_send_response(request, OC_STATUS_BAD_REQUEST);
      return 1;
    }
  }
  return 2;
}

static void
set_semaphore(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  semaphore_t *semaphore = (semaphore_t *)user_data;
  (void)iface_mask;
  oc_rep_t *rep = request->request_payload;
  while (rep != NULL) {
    char *key = oc_string(rep->name);

    uint8_t result;
    if((result = set_semaphore_value_from_resp(request, rep, "green", key, &semaphore->green)) != 2){
      if(result == 1)
        return;
    }
    else if((result = set_semaphore_value_from_resp(request, rep, "yellow", key, &semaphore->yellow)) != 2){
      if(result == 1)
        return;
    }
    else if((result = set_semaphore_value_from_resp(request, rep, "red", key, &semaphore->red)) != 2){
      if(result == 1)
        return;
    }

    rep = rep->next;
  }
  mf_hw_semaphore_set_value(semaphore->green, semaphore->yellow, semaphore->red);
  oc_send_response(request, OC_STATUS_CHANGED);
}


uint8_t mf_semaphore_create_resource(oc_resource_t *semaph_resource){
  oc_resource_bind_resource_type(semaph_resource, "mf.r.semaphore");
  oc_resource_bind_resource_interface(semaph_resource, OC_IF_BASELINE);
  oc_resource_bind_resource_interface(semaph_resource, OC_IF_A);
  oc_resource_set_default_interface(semaph_resource, OC_IF_A);
  oc_resource_set_request_handler(semaph_resource, OC_GET, get_semaphore, &semaphore);
  oc_resource_set_request_handler(semaph_resource, OC_POST, set_semaphore, &semaphore);
  

  return 0;
}

uint8_t mf_semaphore_init(void){
  return mf_hw_semaphore_init();
}

uint8_t mf_semaphore_destroy(void){
  return mf_hw_semaphore_destroy();
}

