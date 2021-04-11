
#include <stdbool.h>
#include "mf_switch_actuator.h"
#include "mf_log.h"
#include "mf_oc_helpers.h"


#include "driver/gpio.h"
#define btoa(x) ((x) ? "true" : "false")

#define BLINK_GPIO 13 //CONFIG_BLINK_GPIO

static char *g_binaryswitch_RESOURCE_PROPERTY_NAME_value = "value";                                  /* the name for the attribute */
bool g_binaryswitch_value = false; /* current value of property "value" The status of the switch. */ /* registration data variables for the resources */

/**
* get method for "/binaryswitch" resource.
* function is called to intialize the return values of the GET method.
* initialisation of the returned values are done from the global property values.
* Resource Description:
* This Resource describes a binary switch (on/off).
* The Property "value" is a boolean.
* A value of 'true' means that the switch is on.
* A value of 'false' means that the switch is off.
*
* @param request the request representation.
* @param interfaces the interface used for this call
* @param user_data the user data.
*/
static void
get_switch_actuator(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)user_data; /* variable not used */
  /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
     the call to the HW needs to fill in the global variable before it returns to this function here.
     alternative is to have a callback from the hardware that sets the global variables.
  
     The implementation always return everything that belongs to the resource.
     this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
  bool error_state = false;

  oc_rep_start_root_object();
  switch (interfaces)
  {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
    /* fall through */
  case OC_IF_A:

    /* property (boolean) 'value' */
    oc_rep_set_boolean(root, value, g_binaryswitch_value);
   // PRINT("   %s : %s\n", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  if (error_state == false)
  {
    oc_send_response(request, OC_STATUS_OK);
  }
  else
  {
    oc_send_response(request, OC_STATUS_BAD_OPTION);
  }
}

/**
* post method for "/binaryswitch" resource.
* The function has as input the request body, which are the input values of the POST method.
* The input values (as a set) are checked if all supplied values are correct.
* If the input values are correct, they will be assigned to the global  property values.
* Resource Description:

*
* @param request the request representation.
* @param interfaces the used interfaces during the request.
* @param user_data the supplied user data.
*/
static void
post_switch_actuator(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)interfaces;
  (void)user_data;
  bool error_state = false;
  PRINT("-- Begin post_binaryswitch:\n");
  oc_rep_t *rep = request->request_payload;

  /* loop over the request document for each required input field to check if all required input fields are present */
  bool var_in_request = false;
  rep = request->request_payload;
  while (rep != NULL)
  {
    if (strcmp(oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value) == 0)
    {
      var_in_request = true;
    }
    rep = rep->next;
  }
  if (var_in_request == false)
  {
    error_state = true;
    PRINT(" required property: 'value' not in request\n");
  }
  /* loop over the request document to check if all inputs are ok */
  rep = request->request_payload;
  while (rep != NULL)
  {
    PRINT("key: (check) %s \n", oc_string(rep->name));

    error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
    if (strcmp(oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value) == 0)
    {
      /* property "value" of type boolean exist in payload */
      if (rep->type != OC_REP_BOOL)
      {
        error_state = true;
        PRINT("   property 'value' is not of type bool %d \n", rep->type);
      }
    }
    rep = rep->next;
  }
  /* if the input is ok, then process the input document and assign the global variables */
  if (error_state == false)
  {
    switch (interfaces)
    {
    default:
    {
      /* loop over all the properties in the input document */
      oc_rep_t *rep = request->request_payload;
      while (rep != NULL)
      {
        PRINT("key: (assign) %s \n", oc_string(rep->name));
        /* no error: assign the variables */

        if (strcmp(oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value) == 0)
        {
          /* assign "value" */
          PRINT("  property 'value' : %s\n", (char *)btoa(rep->value.boolean));
          g_binaryswitch_value = rep->value.boolean;
        }
        rep = rep->next;
      }
      /* set the response */
      PRINT("Set response \n");
      oc_rep_start_root_object();
      /*oc_process_baseline_interface(request->resource); */
      PRINT("   %s : %s", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
      oc_rep_set_boolean(root, value, g_binaryswitch_value);

      oc_rep_end_root_object();
      gpio_set_level(BLINK_GPIO, g_binaryswitch_value);
      oc_send_response(request, OC_STATUS_CHANGED);
    }
    }
  }
  else
  {
    PRINT("  Returning Error \n");
    /* TODO: add error response, if any */
    //oc_send_response(request, OC_STATUS_NOT_MODIFIED);
    oc_send_response(request, OC_STATUS_BAD_REQUEST);
  }
  PRINT("-- End post_binaryswitch\n");
}


uint8_t mf_switch_actuator_create_resource(oc_resource_t *switch_res){
  oc_resource_bind_resource_type(switch_res, "oic.r.temperature");
    oc_resource_bind_resource_interface(switch_res, OC_IF_BASELINE);
  oc_resource_bind_resource_interface(switch_res, OC_IF_A);
  oc_resource_set_default_interface(switch_res, OC_IF_A);
  oc_resource_set_request_handler(switch_res, OC_POST, post_switch_actuator, NULL);
  oc_resource_set_request_handler(switch_res, OC_GET, get_switch_actuator, NULL);
  oc_resource_tag_func_desc(switch_res, OC_ENUM_HEATING);
  oc_resource_tag_pos_desc(switch_res, OC_POS_CENTRE);

  return 0;
}

uint8_t mf_switch_actuator_init(void){
  gpio_reset_pin(BLINK_GPIO);
  gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(BLINK_GPIO, g_binaryswitch_value);
  
  return 0;
}

uint8_t mf_switch_actuator_destroy(void){
  return 0;
}

