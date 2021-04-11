
#include <stdbool.h>
#include "mf_ultrasound.h"
#include "mf_hw_ultrasound.h"

float g_distance_measurement = 0; /* current value of property "measurement"  Measured value for this sensor, units depend on the specific type of sensor */
static char *g_distance_RESOURCE_PROPERTY_NAME_unit = "unit"; /* the name for the attribute */
char g_distance_unit[] = "cm"; /* current value of property "unit" SI unit in SenML of the measurement *//* registration data variables for the resources */


/**
* get method for "/distance" resource.
* function is called to intialize the return values of the GET method.
* initialisation of the returned values are done from the global property values.
* Resource Description:
* This Resource describes a continuous measurement of some value or property or entity .
* The Property "measurement" is a number.
*  The Property unit is a string and will contain an SI unit of measurement in senML format  https://www.iana.org/assignments/senml/senml.xhtml
*
* @param request the request representation.
* @param interfaces the interface used for this call
* @param user_data the user data.
*/
static void
get_distance(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)user_data;  /* variable not used */
  
  mf_hw_ultrasound_get_value(&g_distance_measurement);
  
               //   PRINT("RESULT: %f\n", g_distance_measurement);

  bool error_state = false;
  
  
 // PRINT("-- Begin get_distance: interface %d\n", interfaces);
  oc_rep_start_root_object();
  switch (interfaces) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);    
    /* fall through */
  case OC_IF_S:
    /* property (number) 'measurement' */
    oc_rep_set_double(root, measurement, g_distance_measurement);
    /* property (string) 'unit' */
    oc_rep_set_text_string(root, unit, g_distance_unit);
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  if (error_state == false) {
    oc_send_response(request, OC_STATUS_OK);
  }
  else {
    oc_send_response(request, OC_STATUS_BAD_OPTION);
  }
}
  



uint8_t mf_ultrasound_create_resource(oc_resource_t *ultrasound_resource){
  oc_resource_bind_resource_type(ultrasound_resource, "oic.r.sensor.measurement");
  oc_resource_bind_resource_interface(ultrasound_resource,  OC_IF_S); /* oic.if.s */
  oc_resource_bind_resource_interface(ultrasound_resource,  OC_IF_BASELINE); /* oic.if.baseline */
  oc_resource_set_default_interface(ultrasound_resource,  OC_IF_S);  
  oc_resource_set_request_handler(ultrasound_resource, OC_GET, get_distance, NULL);
  return 0;
}

uint8_t mf_ultrasound_init(void){
  return mf_hw_ultrasound_init();
}

uint8_t mf_ultrasound_destroy(void){
  return mf_hw_ultrasound_destroy();
}


