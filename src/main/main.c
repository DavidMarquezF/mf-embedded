/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 Copyright 2017-2021 Open Connectivity Foundation
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

/* Application Design
*
* support functions:
* app_init
*  initializes the oic/p and oic/d values.
* register_resources
*  function that registers all endpoints, e.g. sets the RETRIEVE/UPDATE handlers for each end point
*
* main 
*  starts the stack, with the registered resources.
*
* Each resource has:
*  global property variables (per resource path) for:
*    the property name
*       naming convention: g_<path>_RESOURCE_PROPERTY_NAME_<propertyname>
*    the actual value of the property, which is typed from the json data type
*      naming convention: g_<path>_<propertyname>
*  global resource variables (per path) for:
*    the path in a variable:
*      naming convention: g_<path>_RESOURCE_ENDPOINT
*
*  handlers for the implemented methods (get/post)
*   get_<path>
*     function that is being called when a RETRIEVE is called on <path>
*     set the global variables in the output
*   post_<path>
*     function that is being called when a UPDATE is called on <path>
*     checks the input data
*     if input data is correct
*       updates the global variables
*
*/
/*
 tool_version          : 20200103
 input_file            : /home/david/workspace_iot/example/device_output/out_codegeneration_merged.swagger.json
 version of input_file :  
 title of input_file   : Switch
*/

#include "oc_api.h"
#include "port/oc_clock.h"
#include <signal.h>

#ifdef OC_CLOUD
#include "oc_cloud.h"
#endif
#if defined(OC_IDD_API)
#include "oc_introspection.h"
#endif

#if defined(OC_SECURITY) && defined(OC_PKI)
/* code to include an pki certificate and root trust anchor */
#include "oc_pki.h"
#endif
#include "mf_temp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "debug_print.h"

#include <pthread.h>
#include <stdio.h>
#include <inttypes.h>

#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD
#define BLINK_GPIO 13 //CONFIG_BLINK_GPIO
#define STACK_SIZE 20000


#define btoa(x) ((x) ? "true" : "false")

#define MAX_STRING 30         /* max size of the strings. */
#define MAX_PAYLOAD_STRING 65 /* max size strings in the payload */
#define MAX_ARRAY 10          /* max size of the array */
/* Note: Magic numbers are derived from the resource definition, either from the example or the definition.*/

static const char *TAG = "iotivity server";

static int quit = 0; /* stop variable, used by handle_signal */
static EventGroupHandle_t wifi_event_group;

static const int IPV4_CONNECTED_BIT = BIT0;
static const int IPV6_CONNECTED_BIT = BIT1;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static struct timespec ts;

static TaskHandle_t xHandle = NULL;
// Structure that will hold the TCB of the task being created.
static StaticTask_t xTaskBuffer;

// Buffer that the task being created will use as its stack.  Note this is
// an array of StackType_t variables.  The size of StackType_t is dependent on
// the RTOS port.
static StackType_t xStack[STACK_SIZE];

/* global property variables for path: "/binaryswitch" */
static char *g_binaryswitch_RESOURCE_PROPERTY_NAME_value = "value";                                  /* the name for the attribute */
bool g_binaryswitch_value = false; /* current value of property "value" The status of the switch. */ /* registration data variables for the resources */

/* global resource variables for path: /binaryswitch */
static char *g_binaryswitch_RESOURCE_ENDPOINT = "/binaryswitch";                 /* used path for this resource */
static char *g_binaryswitch_RESOURCE_TYPE[MAX_STRING] = {"oic.r.switch.binary"}; /* rt value (as an array) */
int g_binaryswitch_nr_resource_types = 1;

/* global property variables for path: "/freezer1" */
static char *g_freezer1_RESOURCE_PROPERTY_NAME_precision = "precision"; /* the name for the attribute */
double g_freezer1_precision = 0;                                        /* current value of property "precision"  When exposed the value in 'precision' provides a +/- tolerance against the Properties in the Resource. Thus if a Property is UP\
DATED to a value and that Property then RETRIEVED, the RETRIEVED value is valid if in the range of the set value +/- precision */
static char *g_freezer1_RESOURCE_PROPERTY_NAME_range = "range";         /* the name for the attribute */

/* array range  The valid range for the Property in the Resource as a number. The first value in the array is the minimum value, the second value in the array is the maximum value. */
double g_freezer1_range[2];
size_t g_freezer1_range_array_size;

static char *g_freezer1_RESOURCE_PROPERTY_NAME_step = "step";               /* the name for the attribute */
double g_freezer1_step = 0;                                                 /* current value of property "step"  Step value across the defined range an integer when the range is a number.  This is the increment for valid values across the range; so i\
f range is 0.0..10.0 and step is 2.5 then valid values are 0.0,2.5,5.0,7.5,10.0. */
static char *g_freezer1_RESOURCE_PROPERTY_NAME_temperature = "temperature"; /* the name for the attribute */
double g_freezer1_temperature = 0;                                          /* current value of property "temperature"  The current temperature setting or measurement. */
static char *g_freezer1_RESOURCE_PROPERTY_NAME_units = "units";             /* the name for the attribute */
char g_freezer1_units[MAX_PAYLOAD_STRING] = ""
                                            ""; /* current value of property "units" The unit for the conveyed temperature value, Note that when doing an UPDATE, the unit on the device does NOT chan\
ge, it only indicates the unit of the conveyed value during the UPDATE operation. */
/* global property variables for path: "/light1" */
static char *g_light1_RESOURCE_PROPERTY_NAME_value = "value";                                  /* the name for the attribute */
bool g_light1_value = false; /* current value of property "value" The status of the switch. */ /* registration data variables for the resources */

/* global resource variables for path: /freezer1 */
static char *g_freezer1_RESOURCE_ENDPOINT = "/freezer1";                   /* used path for this resource */
static char *g_freezer1_RESOURCE_TYPE[MAX_STRING] = {"oic.r.temperature"}; /* rt value (as an array) */
int g_freezer1_nr_resource_types = 1;

// Create DS18B20 devices on the 1-Wire bus
//static  DS18B20_Info *tempDevice;
//static  OneWireBus *owb;

/**
* function to set up the device.
*
*/
int app_init(void)
{
  int ret = oc_init_platform("ocf", NULL, NULL);
  /* the settings determine the appearance of the device on the network
     can be ocf.2.2.0 (or even higher)
     supplied values are for ocf.2.2.0 */
  ret |= oc_add_device("/oic/d", "oic.d.switchdevice", "SwitchESP",
                       "ocf.2.2.0",                   /* icv value */
                       "ocf.res.1.3.0, ocf.sh.1.3.0", /* dmv value */
                       NULL, NULL);

#if defined(OC_IDD_API)
  FILE *fp;
  uint8_t *buffer;
  size_t buffer_size;
  const char introspection_error[] =
      "\tERROR Could not read 'server_introspection.cbor'\n"
      "\tIntrospection data not set.\n";
  fp = fopen("./server_introspection.cbor", "rb");
  if (fp)
  {
    fseek(fp, 0, SEEK_END);
    buffer_size = ftell(fp);
    rewind(fp);

    buffer = (uint8_t *)malloc(buffer_size * sizeof(uint8_t));
    size_t fread_ret = fread(buffer, buffer_size, 1, fp);
    fclose(fp);

    if (fread_ret == 1)
    {
      oc_set_introspection_data(0, buffer, buffer_size);
      PRINT("\tIntrospection data set 'server_introspection.cbor': %d [bytes]\n", (int)buffer_size);
    }
    else
    {
      PRINT("%s", introspection_error);
    }
    free(buffer);
  }
  else
  {
    PRINT("%s", introspection_error);
  }
#else
  PRINT("\t introspection via header file\n");
#endif
  return ret;
}

/**
* helper function to check if the POST input document contains 
* the common readOnly properties or the resouce readOnly properties
* @param name the name of the property
* @return the error_status, e.g. if error_status is true, then the input document contains something illegal
*/
static bool
check_on_readonly_common_resource_properties(oc_string_t name, bool error_state)
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

static void sta_start(void *esp_netif, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
  esp_wifi_connect();
}

static void sta_disconnected(void *esp_netif, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
  esp_wifi_connect();
  xEventGroupClearBits(wifi_event_group, IPV4_CONNECTED_BIT);
  xEventGroupClearBits(wifi_event_group, IPV6_CONNECTED_BIT);
}

static void sta_connected(void *esp_netif, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  esp_netif_create_ip6_linklocal(esp_netif);
}

static void got_ip(void *esp_netif, esp_event_base_t event_base,
                   int32_t event_id, void *event_data)
{
  xEventGroupSetBits(wifi_event_group, IPV4_CONNECTED_BIT);
}

static void got_ip6(void *esp_netif, esp_event_base_t event_base,
                    int32_t event_id, void *event_data)
{
  xEventGroupSetBits(wifi_event_group, IPV6_CONNECTED_BIT);
}

static void initialise_wifi(void)
{
  esp_err_t err = esp_event_loop_create_default();
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
  {
    ESP_ERROR_CHECK(err);
  }
  ESP_ERROR_CHECK(esp_netif_init());
  char *desc;
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
  // Prefix the interface description with the module TAG
  // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
  asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
  esp_netif_config.if_desc = desc;
  esp_netif_config.route_prio = 128;
  esp_netif_t *netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
  free(desc);
  ESP_ERROR_CHECK(esp_wifi_set_default_wifi_sta_handlers());

  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, sta_disconnected, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, sta_start, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, sta_connected, netif));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, got_ip6, NULL));

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = EXAMPLE_WIFI_SSID,
          .password = EXAMPLE_WIFI_PASS,
      },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
/*
static uint8_t read_temp_sensor(float* result)
{
  if(!owb || !tempDevice){
    PRINT("ERROR obtaining temp");
    return 1;
  }

  // Read the results immediately after conversion otherwise it may fail
  // (using printf before reading may take too long)
  float readings = 0;
  DS18B20_ERROR errors = 0;
// In this application all devices use the same resolution,
  // so use the first device to determine the delay
  errors = ds18b20_convert_and_read_temp(tempDevice, &readings);

    
  // Print results in a separate loop, after all have been read
  PRINT("\nTemperature readings (degrees C)\n");
    if (errors != DS18B20_OK)
    {
    PRINT("Error reading temp");
    return 1;
    }

    PRINT("  Temp: %.1f\n", readings);
    *result = readings;
    return 0;
  
}
static owb_rmt_driver_info rmt_driver_info;
*/
// Code from https://github.com/DavidAntliff/esp32-ds18b20-example/blob/master/main/app_main.c
/*static void setup_temp_sensor(void)
{
  // Create a 1-Wire bus, using the RMT timeslot driver
  owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
  owb_use_crc(owb, true); // enable CRC check for ROM code

  // Find all connected devices
  PRINT("Find devices:\n");
  OneWireBus_SearchState search_state = {0};
  bool found = false;
  owb_search_first(owb, &search_state, &found);
  
  OneWireBus_ROMCode rom_code;
  owb_status status = owb_read_rom(owb, &rom_code);
  if (status == OWB_STATUS_OK)
  {
    char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
    owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
    PRINT("Single device %s present\n", rom_code_s);
  }
  else
  {
    PRINT("An error occurred reading ROM code: %d", status);
  }

  tempDevice =  ds18b20_malloc(); // heap allocation

  PRINT("Single device optimisations enabled\n");
  ds18b20_init_solo(tempDevice, owb); // only one device on bus
  ds18b20_use_crc(tempDevice, true);  // enable CRC check on all reads
  ds18b20_set_resolution(tempDevice, DS18B20_RESOLUTION);

  // Check for parasitic-powered devices
  bool parasitic_power = false;
  ds18b20_check_for_parasite_power(owb, &parasitic_power);
  if (parasitic_power)
  {
    PRINT("Parasitic-powered devices detected");
  }

  // In parasitic-power mode, devices cannot indicate when conversions are complete,
  // so waiting for a temperature conversion must be done by waiting a prescribed duration
  owb_use_parasitic_power(owb, parasitic_power);

#ifdef CONFIG_ENABLE_STRONG_PULLUP_GPIO
  // An external pull-up circuit is used to supply extra current to OneWireBus devices
  // during temperature conversions.
  owb_use_strong_pullup_gpio(owb, CONFIG_STRONG_PULLUP_GPIO);
#endif
}
*/

static void
get_freezer1(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)user_data; /* variable not used */
  float tempResult = 0;
  mf_temp_get_value(&tempResult);
  g_freezer1_temperature =(double)tempResult;
  /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.                                                                                                                          
     the call to the HW needs to fill in the global variable before it returns to this function here.                                                                                                      
     alternative is to have a callback from the hardware that sets the global variables.                                                                                                                   
                                                                                                                                                                                                           
     The implementation always return everything that belongs to the resource.                                                                                                                             
     this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
  bool error_state = false;

  /* query name 'units' type: 'string', enum: ['C', 'F', 'K']*/
  char *_units = NULL; /* not null terminated Units */
  int _units_len = oc_get_query_value(request, "units", &_units);
  if (_units_len != -1)
  {
    PRINT(" query value 'units': %.*s\n", _units_len, _units);
    bool query_ok = false;
    /* input check ['C', 'F', 'K']  */

    if (strncmp(_units, "C", _units_len) == 0)
      query_ok = true;
    if (strncmp(_units, "F", _units_len) == 0)
      query_ok = true;
    if (strncmp(_units, "K", _units_len) == 0)
      query_ok = true;
    if (query_ok == false)
      error_state = true;
    /* TODO: use the query value to tailer the response*/
  }

  PRINT("-- Begin get_freezer1: interface %d\n", interfaces);
  oc_rep_start_root_object();
  switch (interfaces)
  {
  case OC_IF_BASELINE:
    PRINT("   Adding Baseline info\n");
    oc_process_baseline_interface(request->resource);

    /* property (number) 'precision' */
    oc_rep_set_double(root, precision, g_freezer1_precision);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_precision, g_freezer1_precision);
    /* property (array of numbers) 'range' */
    oc_rep_set_array(root, range);
    PRINT("   %s double = [ ", g_freezer1_RESOURCE_PROPERTY_NAME_range);
    for (int i = 0; i < (int)g_freezer1_range_array_size; i++)
    {
      oc_rep_add_double(range, g_freezer1_range[i]);
      PRINT("   %f ", g_freezer1_range[i]);
    }
    PRINT("   ]\n");
    oc_rep_close_array(root, range);
    /* property (number) 'step' */
    oc_rep_set_double(root, step, g_freezer1_step);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_step, g_freezer1_step);
    /* property (number) 'temperature' */
    oc_rep_set_double(root, temperature, g_freezer1_temperature);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_temperature, g_freezer1_temperature);
    /* property (string) 'units' */
    oc_rep_set_text_string(root, units, g_freezer1_units);
    PRINT("   %s : %s\n", g_freezer1_RESOURCE_PROPERTY_NAME_units, g_freezer1_units);
    break;
  case OC_IF_S:

    /* property (number) 'precision' */
    oc_rep_set_double(root, precision, g_freezer1_precision);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_precision, g_freezer1_precision);
    /* property (array of numbers) 'range' */
    oc_rep_set_array(root, range);
    PRINT("   %s double = [ ", g_freezer1_RESOURCE_PROPERTY_NAME_range);
    for (int i = 0; i < (int)g_freezer1_range_array_size; i++)
    {
      oc_rep_add_double(range, g_freezer1_range[i]);
      PRINT("   %f ", g_freezer1_range[i]);
    }
    PRINT("   ]\n");
    oc_rep_close_array(root, range);
    /* property (number) 'step' */
    oc_rep_set_double(root, step, g_freezer1_step);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_step, g_freezer1_step);
    /* property (number) 'temperature' */
    oc_rep_set_double(root, temperature, g_freezer1_temperature);
    PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_temperature, g_freezer1_temperature);
    /* property (string) 'units' */
    oc_rep_set_text_string(root, units, g_freezer1_units);
    PRINT("   %s : %s\n", g_freezer1_RESOURCE_PROPERTY_NAME_units, g_freezer1_units);
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
  PRINT("-- End get_freezer1\n");
}

/**                                                                                                                                                                                                        
* post method for "/freezer1" resource.                                                                                                                                                                    
* The function has as input the request body, which are the input values of the POST method.                                                                                                               
* The input values (as a set) are checked if all supplied values are correct.                                                                                                                              
* If the input values are correct, they will be assigned to the global  property values.                                                                                                                   
* Resource Description:                                                                                                                                                                                    
* Sets the desired temperature value.                                                                                                                                                                      
* If a "unit" is included and the server may not support the unit indicated the request will fail.                                                                                                         
* If the units are omitted value is taken to be in C.                                                                                                                                                      
*                                                                                                                                                                                                          
* @param request the request representation.                                                                                                                                                               
* @param interfaces the used interfaces during the request.                                                                                                                                                
* @param user_data the supplied user data.                                                                                                                                                                 
*/
static void
post_freezer1(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)interfaces;
  (void)user_data;
  bool error_state = false;
  PRINT("-- Begin post_freezer1:\n");
  oc_rep_t *rep = request->request_payload;

  /* loop over the request document for each required input field to check if all required input fields are present */
  bool var_in_request = false;
  rep = request->request_payload;
  while (rep != NULL)
  {
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_temperature) == 0)
    {
      var_in_request = true;
    }
    rep = rep->next;
  }
  if (var_in_request == false)
  {
    error_state = true;
    PRINT(" required property: 'temperature' not in request\n");
  }
  /* loop over the request document to check if all inputs are ok */
  rep = request->request_payload;
  while (rep != NULL)
  {
    PRINT("key: (check) %s \n", oc_string(rep->name));

    error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_precision) == 0)
    {
      /* property "precision" of type double exist in payload */
      /* check if "precision" is read only */
      error_state = true;
      PRINT("   property 'precision' is readOnly \n");
      if ((rep->type != OC_REP_DOUBLE) & (rep->type != OC_REP_INT))
      {
        error_state = true;
        PRINT("   property 'precision' is not of type double or int %d \n", rep->type);
      }
    }
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_range) == 0)
    {
      /* property "range" of type array exist in payload */
      /* check if "range" is read only */
      error_state = true;
      PRINT("   property 'range' is readOnly \n");
      size_t array_size = 0;

      double *temp_array = 0;
      oc_rep_get_double_array(rep, "range", &temp_array, &array_size);

      if (array_size > 2)
      {
        error_state = true;
        PRINT("   property array 'range' is too long: %d expected: 2 \n", (int)array_size);
      }
    }
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_step) == 0)
    {
      /* property "step" of type double exist in payload */
      /* check if "step" is read only */
      error_state = true;
      PRINT("   property 'step' is readOnly \n");
      if ((rep->type != OC_REP_DOUBLE) & (rep->type != OC_REP_INT))
      {
        error_state = true;
        PRINT("   property 'step' is not of type double or int %d \n", rep->type);
      }
    }
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_temperature) == 0)
    {
      /* property "temperature" of type double exist in payload */
      if ((rep->type != OC_REP_DOUBLE) & (rep->type != OC_REP_INT))
      {
        error_state = true;
        PRINT("   property 'temperature' is not of type double or int %d \n", rep->type);
      }
    }
    if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_units) == 0)
    {
      /* property "units" of type string exist in payload */
      if (rep->type != OC_REP_STRING)
      {
        error_state = true;
        PRINT("   property 'units' is not of type string %d \n", rep->type);
      }
      if (strlen(oc_string(rep->value.string)) >= (MAX_PAYLOAD_STRING - 1))
      {
        error_state = true;
        PRINT("   property 'units' is too long %d expected: MAX_PAYLOAD_STRING-1 \n", (int)strlen(oc_string(rep->value.string)));
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

        if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_precision) == 0)
        {
          /* assign "precision" */
          PRINT("  property 'precision' : %f\n", rep->value.double_p);
          g_freezer1_precision = rep->value.double_p;
        }
        if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_range) == 0)
        {
          /* retrieve the array pointer to the double array of property "range"                                                                                                                        
                 note that the variable g_freezer1_range_array_size will contain the array size in the payload. */
          double *temp_double = 0;
          oc_rep_get_double_array(rep, "range", &temp_double, &g_freezer1_range_array_size);
          /* copy over the data of the retrieved array to the global variable */
          for (int j = 0; j < (int)g_freezer1_range_array_size; j++)
          {
            PRINT(" double %f ", temp_double[j]);
            g_freezer1_range[j] = temp_double[j];
          }
        }
        if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_step) == 0)
        {
          /* assign "step" */
          PRINT("  property 'step' : %f\n", rep->value.double_p);
          g_freezer1_step = rep->value.double_p;
        }
        if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_temperature) == 0)
        {
          /* assign "temperature" */
          PRINT("  property 'temperature' : %f\n", rep->value.double_p);
          g_freezer1_temperature = rep->value.double_p;
        }
        if (strcmp(oc_string(rep->name), g_freezer1_RESOURCE_PROPERTY_NAME_units) == 0)
        {
          /* assign "units" */
          PRINT("  property 'units' : %s\n", oc_string(rep->value.string));
          strncpy(g_freezer1_units, oc_string(rep->value.string), MAX_PAYLOAD_STRING - 1);
        }
        rep = rep->next;
      }
      /* set the response */
      PRINT("Set response \n");
      oc_rep_start_root_object();
      /*oc_process_baseline_interface(request->resource); */
      PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_precision, g_freezer1_precision);
      oc_rep_set_double(root, precision, g_freezer1_precision);

      oc_rep_set_array(root, range);
      for (int i = 0; i < (int)g_freezer1_range_array_size; i++)
      {
        oc_rep_add_double(range, g_freezer1_range[i]);
      }
      oc_rep_close_array(root, range);

      PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_step, g_freezer1_step);
      oc_rep_set_double(root, step, g_freezer1_step);
      PRINT("   %s : %f\n", g_freezer1_RESOURCE_PROPERTY_NAME_temperature, g_freezer1_temperature);
      oc_rep_set_double(root, temperature, g_freezer1_temperature);
      PRINT("   %s : %s\n", g_freezer1_RESOURCE_PROPERTY_NAME_units, g_freezer1_units);
      oc_rep_set_text_string(root, units, g_freezer1_units);

      oc_rep_end_root_object();
      /* TODO: ACTUATOR add here the code to talk to the HW if one implements an actuator.                                                                                                                 
       one can use the global variables as input to those calls                                                                                                                                            
       the global values have been updated already with the data from the request */
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
  PRINT("-- End post_freezer1\n");
}

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
get_binaryswitch(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
  (void)user_data; /* variable not used */
  /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
     the call to the HW needs to fill in the global variable before it returns to this function here.
     alternative is to have a callback from the hardware that sets the global variables.
  
     The implementation always return everything that belongs to the resource.
     this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
  bool error_state = false;

  PRINT("-- Begin get_binaryswitch: interface %d\n", interfaces);
  oc_rep_start_root_object();
  switch (interfaces)
  {
  case OC_IF_BASELINE:
    PRINT("   Adding Baseline info\n");
    oc_process_baseline_interface(request->resource);

    /* property (boolean) 'value' */
    oc_rep_set_boolean(root, value, g_binaryswitch_value);
    PRINT("   %s : %s\n", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
    break;
  case OC_IF_A:

    /* property (boolean) 'value' */
    oc_rep_set_boolean(root, value, g_binaryswitch_value);
    PRINT("   %s : %s\n", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
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
  PRINT("-- End get_binaryswitch\n");
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
post_binaryswitch(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
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

/**
* register all the resources to the stack
* this function registers all application level resources:
* - each resource path is bind to a specific function for the supported methods (GET, POST, PUT)
* - each resource is 
*   - secure
*   - observable
*   - discoverable 
*   - used interfaces, including the default interface.
*     default interface is the first of the list of interfaces as specified in the input file
*/
void register_resources(void)
{
  PRINT("Register Resource with local path \"/freezer1\"\n");
  oc_resource_t *res_freezer1 = oc_new_resource(NULL, g_freezer1_RESOURCE_ENDPOINT, g_freezer1_nr_resource_types, 0);
  PRINT("     number of Resource Types: %d\n", g_freezer1_nr_resource_types);
  for (int a = 0; a < g_freezer1_nr_resource_types; a++)
  {
    PRINT("     Resource Type: \"%s\"\n", g_freezer1_RESOURCE_TYPE[a]);
    oc_resource_bind_resource_type(res_freezer1, g_freezer1_RESOURCE_TYPE[a]);
  }

  oc_resource_bind_resource_interface(res_freezer1, OC_IF_BASELINE); /* oic.if.baseline */
  oc_resource_bind_resource_interface(res_freezer1, OC_IF_A);        /* oic.if.a */
  oc_resource_bind_resource_interface(res_freezer1, OC_IF_S);        /* oic.if.s */
  oc_resource_set_default_interface(res_freezer1, OC_IF_BASELINE);
  PRINT("     Default OCF Interface: 'oic.if.baseline'\n");
  oc_resource_set_discoverable(res_freezer1, true);
  /* periodic observable                                                                                                                                                                                   
     to be used when one wants to send an event per time slice                                                                                                                                             
     period is 1 second */
  oc_resource_set_periodic_observable(res_freezer1, 1);
  /* set observable                                                                                                                                                                                        
     events are send when oc_notify_observers(oc_resource_t *resource) is called.                                                                                                                          
    this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
  /*oc_resource_set_observable(res_freezer1, true); */

  oc_resource_set_request_handler(res_freezer1, OC_GET, get_freezer1, NULL);

  oc_resource_set_request_handler(res_freezer1, OC_POST, post_freezer1, NULL);
#ifdef OC_CLOUD
  oc_cloud_add_resource(res_freezer1);
#endif

  oc_add_resource(res_freezer1);

  PRINT("Register Resource with local path \"/binaryswitch\"\n");
  oc_resource_t *res_binaryswitch = oc_new_resource(NULL, g_binaryswitch_RESOURCE_ENDPOINT, g_binaryswitch_nr_resource_types, 0);
  PRINT("     number of Resource Types: %d\n", g_binaryswitch_nr_resource_types);
  for (int a = 0; a < g_binaryswitch_nr_resource_types; a++)
  {
    PRINT("     Resource Type: \"%s\"\n", g_binaryswitch_RESOURCE_TYPE[a]);
    oc_resource_bind_resource_type(res_binaryswitch, g_binaryswitch_RESOURCE_TYPE[a]);
  }

  oc_resource_bind_resource_interface(res_binaryswitch, OC_IF_A);        /* oic.if.a */
  oc_resource_bind_resource_interface(res_binaryswitch, OC_IF_BASELINE); /* oic.if.baseline */
  oc_resource_set_default_interface(res_binaryswitch, OC_IF_A);
  PRINT("     Default OCF Interface: 'oic.if.a'\n");
  oc_resource_set_discoverable(res_binaryswitch, true);
  /* periodic observable
     to be used when one wants to send an event per time slice
     period is 1 second */
  oc_resource_set_periodic_observable(res_binaryswitch, 1);
  /* set observable
     events are send when oc_notify_observers(oc_resource_t *resource) is called.
    this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
  /*oc_resource_set_observable(res_binaryswitch, true); */

  oc_resource_set_request_handler(res_binaryswitch, OC_GET, get_binaryswitch, NULL);

  oc_resource_set_request_handler(res_binaryswitch, OC_POST, post_binaryswitch, NULL);

#ifdef OC_CLOUD
  oc_cloud_add_resource(res_binaryswitch);
#endif
  oc_add_resource(res_binaryswitch);
}

#if defined(OC_SECURITY) && defined(OC_PKI)
static int set_root_certificate(void)
{
  const char *root_ca = "-----BEGIN CERTIFICATE-----\n"
                        "MIIBaTCCAQ+gAwIBAgIQR33gIB75I7Vi/QnMnmiWvzAKBggqhkjOPQQDAjATMREw\n"
                        "DwYDVQQKEwhUZXN0IE9SRzAeFw0xOTA1MDIyMDA1MTVaFw0yOTAzMTAyMDA1MTVa\n"
                        "MBMxETAPBgNVBAoTCFRlc3QgT1JHMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE\n"
                        "xbwMaS8jcuibSYJkCmuVHfeV3xfYVyUq8Iroz7YlXaTayspW3K4hVdwIsy/5U+3U\n"
                        "vM/vdK5wn2+NrWy45vFAJqNFMEMwDgYDVR0PAQH/BAQDAgEGMBMGA1UdJQQMMAoG\n"
                        "CCsGAQUFBwMBMA8GA1UdEwEB/wQFMAMBAf8wCwYDVR0RBAQwAoIAMAoGCCqGSM49\n"
                        "BAMCA0gAMEUCIBWkxuHKgLSp6OXDJoztPP7/P5VBZiwLbfjTCVRxBvwWAiEAnzNu\n"
                        "6gKPwtKmY0pBxwCo3NNmzNpA6KrEOXE56PkiQYQ=\n"
                        "-----END CERTIFICATE-----\n";
  return oc_pki_add_mfg_trust_anchor(0, (const unsigned char *)root_ca, strlen(root_ca));
}
#endif /* OC_SECURITY && OC_PKI */

void factory_presets_cb(size_t device, void *data)
{
  (void)device;
  (void)data;
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

#if defined(OC_SECURITY) && defined(OC_PKI)

  // from https://github.com/plgd-dev/cloud/blob/v2/device-simulator/src/apps/cloud_server.c

  const char *cert = "-----BEGIN CERTIFICATE-----\n"
                     "MIIB9zCCAZygAwIBAgIRAOwIWPAt19w7DswoszkVIEIwCgYIKoZIzj0EAwIwEzER\n"
                     "MA8GA1UEChMIVGVzdCBPUkcwHhcNMTkwNTAyMjAwNjQ4WhcNMjkwMzEwMjAwNjQ4\n"
                     "WjBHMREwDwYDVQQKEwhUZXN0IE9SRzEyMDAGA1UEAxMpdXVpZDpiNWEyYTQyZS1i\n"
                     "Mjg1LTQyZjEtYTM2Yi0wMzRjOGZjOGVmZDUwWTATBgcqhkjOPQIBBggqhkjOPQMB\n"
                     "BwNCAAQS4eiM0HNPROaiAknAOW08mpCKDQmpMUkywdcNKoJv1qnEedBhWne7Z0jq\n"
                     "zSYQbyqyIVGujnI3K7C63NRbQOXQo4GcMIGZMA4GA1UdDwEB/wQEAwIDiDAzBgNV\n"
                     "HSUELDAqBggrBgEFBQcDAQYIKwYBBQUHAwIGCCsGAQUFBwMBBgorBgEEAYLefAEG\n"
                     "MAwGA1UdEwEB/wQCMAAwRAYDVR0RBD0wO4IJbG9jYWxob3N0hwQAAAAAhwR/AAAB\n"
                     "hxAAAAAAAAAAAAAAAAAAAAAAhxAAAAAAAAAAAAAAAAAAAAABMAoGCCqGSM49BAMC\n"
                     "A0kAMEYCIQDuhl6zj6gl2YZbBzh7Th0uu5izdISuU/ESG+vHrEp7xwIhANCA7tSt\n"
                     "aBlce+W76mTIhwMFXQfyF3awWIGjOcfTV8pU\n"
                     "-----END CERTIFICATE-----\n";

  const char *key = "-----BEGIN EC PRIVATE KEY-----\n"
                    "MHcCAQEEIMPeADszZajrkEy4YvACwcbR0pSdlKG+m8ALJ6lj/ykdoAoGCCqGSM49\n"
                    "AwEHoUQDQgAEEuHojNBzT0TmogJJwDltPJqQig0JqTFJMsHXDSqCb9apxHnQYVp3\n"
                    "u2dI6s0mEG8qsiFRro5yNyuwutzUW0Dl0A==\n"
                    "-----END EC PRIVATE KEY-----\n";

  int credid =
      oc_pki_add_mfg_cert(0, (const unsigned char *)cert, strlen(cert), (const unsigned char *)key, strlen(key));
  if (credid < 0)
  {
    PRINT("ERROR installing PKI certificate\n");
    return;
  }
  else
  {
    PRINT("Successfully installed PKI certificate\n");
  }

  /*
  if (oc_pki_add_mfg_intermediate_cert(0, credid, (const unsigned char *)int_ca, strlen(int_ca)) < 0) {
    PRINT("ERROR installing intermediate CA certificate\n");
  } else {
    PRINT("Successfully installed intermediate CA certificate\n");
  }*/

  if (set_root_certificate() < 0)
  {
    PRINT("ERROR installing root certificate\n");
    return;
  }
  else
  {
    PRINT("Successfully installed root certificate\n");
  }

  oc_pki_set_security_profile(0, OC_SP_BLACK, OC_SP_BLACK, credid);

#else
  PRINT("No PKI certificates installed\n");
#endif /* OC_SECURITY && OC_PKI */
}






/**
* intializes the global variables
* registers and starts the handler

*/
void initialize_variables(void)
{
  /* initialize global variables for resource "/binaryswitch" */
  g_binaryswitch_value = false; /* current value of property "value" The status of the switch. */
  gpio_reset_pin(BLINK_GPIO);
  gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(BLINK_GPIO, g_binaryswitch_value);
  mf_temp_setup();

  /* set the flag for NO oic/con resource. */
  oc_set_con_res_announced(false);
   
}

oc_event_callback_retval_t heap_dbg(void *v)
{
  printf("heap size:%d\n", esp_get_free_heap_size());
  return OC_EVENT_CONTINUE;
}

#ifndef NO_MAIN

static void
signal_event_loop(void)
{
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cv);
  pthread_mutex_unlock(&mutex);
}

#ifdef OC_CLOUD
/**
* cloud status handler.
* handler to print out the status of the cloud connection
*/
static void
cloud_status_handler(oc_cloud_context_t *ctx, oc_cloud_status_t status,
                     void *data)
{
  (void)data;
  PRINT("\nCloud Manager Status:\n");
  if (status & OC_CLOUD_REGISTERED)
  {
    PRINT("\t\t-Registered\n");
  }
  if (status & OC_CLOUD_TOKEN_EXPIRY)
  {
    PRINT("\t\t-Token Expiry: ");
    if (ctx)
    {
      PRINT("%d\n", oc_cloud_get_token_expiry(ctx));
    }
    else
    {
      PRINT("\n");
    }
  }
  if (status & OC_CLOUD_FAILURE)
  {
    PRINT("\t\t-Failure\n");
  }
  if (status & OC_CLOUD_LOGGED_IN)
  {
    PRINT("\t\t-Logged In\n");
  }
  if (status & OC_CLOUD_LOGGED_OUT)
  {
    PRINT("\t\t-Logged Out\n");
  }
  if (status & OC_CLOUD_DEREGISTERED)
  {
    PRINT("\t\t-DeRegistered\n");
  }
  if (status & OC_CLOUD_REFRESHED_TOKEN)
  {
    PRINT("\t\t-Refreshed Token\n");
  }
}
#endif // OC_CLOUD

static void loop(void)
{
  oc_clock_time_t next_event;
  while (quit != 1)
  {
    next_event = oc_main_poll();
    pthread_mutex_lock(&mutex);
    if (next_event == 0)
    {
      pthread_cond_wait(&cv, &mutex);
    }
    else
    {
      ts.tv_sec = (next_event / OC_CLOCK_SECOND);
      ts.tv_nsec = (next_event % OC_CLOCK_SECOND) * 1.e09 / OC_CLOCK_SECOND;
      pthread_cond_timedwait(&cv, &mutex, &ts);
    }
    pthread_mutex_unlock(&mutex);
  }
}

static void init(void)
{
  tcpip_adapter_ip_info_t ip4_info = {0};
  struct ip6_addr if_ipaddr_ip6 = {0};
  ESP_LOGI(TAG, "iotivity server task started");
  // wait to fetch IPv4 && ipv6 address
#ifdef OC_IPV4
  xEventGroupWaitBits(wifi_event_group, IPV4_CONNECTED_BIT | IPV6_CONNECTED_BIT, false, true, portMAX_DELAY);
#else
  xEventGroupWaitBits(wifi_event_group, IPV6_CONNECTED_BIT, false, true, portMAX_DELAY);
#endif

#ifdef OC_IPV4
  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip4_info) != ESP_OK)
  {
    print_error("get IPv4 address failed");
  }
  else
  {
    ESP_LOGI(TAG, "got IPv4 addr:%s", ip4addr_ntoa(&(ip4_info.ip)));
  }
#endif

  if (tcpip_adapter_get_ip6_linklocal(TCPIP_ADAPTER_IF_STA, &if_ipaddr_ip6) != ESP_OK)
  {
    print_error("get IPv6 address failed");
  }
  else
  {
    ESP_LOGI(TAG, "got IPv6 addr:%s", ip6addr_ntoa(&if_ipaddr_ip6));
  }
}

/**
* main application.
* intializes the global variables
* registers and starts the handler
* handles (in a loop) the next event.
* shuts down the stack
*/
static void
server_main(void *pvParameter)
{
  init();
  int init;

  PRINT("Used input file : \"/home/david/workspace_iot/example/device_output/out_codegeneration_merged.swagger.json\"\n");
  PRINT("OCF Server name : \"Switch\"\n");

#ifdef OC_SECURITY
  PRINT("Intialize Secure Resources\n");
  oc_storage_config("storage"); //esp32 specific storage

  /*intialize the variables */
  initialize_variables();
  
#endif /* OC_SECURITY */

  /* initializes the handlers structure */
  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop,
                                       .register_resources = register_resources};

  oc_set_factory_presets_cb(factory_presets_cb, NULL);

  // This is a known issue , where max_app_data_size has to be between 6 and 8 Kb to work
  oc_set_max_app_data_size(7168);

  /* start the stack */
  init = oc_main_init(&handler);

  if (init < 0)
  {
    PRINT("oc_main_init failed %d, exiting.\n", init);
    return init;
  }

#ifdef OC_CLOUD
  /* get the cloud context and start the cloud */
  PRINT("Start Cloud Manager\n");
  oc_cloud_context_t *ctx = oc_cloud_get_context(0);
  if (ctx)
  {
    oc_cloud_manager_start(ctx, cloud_status_handler, NULL);
  }
#endif
  oc_set_delayed_callback(NULL, heap_dbg, 1);
  PRINT("OCF server \"Switch\" running, waiting on incoming connections.\n");
  loop();
  /* shut down the stack */
#ifdef OC_CLOUD
  PRINT("Stop Cloud Manager\n");
  oc_cloud_manager_stop(ctx);
#endif
  mf_temp_destroy();
  oc_main_shutdown();
}

void app_main(void)
{
  if (nvs_flash_init() != ESP_OK)
  {
    print_error("nvs_flash_init failed");
  }

  pthread_cond_init(&cv, NULL);

  print_macro_info();

  initialise_wifi();

  // Create the task without using any dynamic memory allocation.
  xHandle = xTaskCreateStatic(
      server_main,   // Function that implements the task.
      "server_main", // Text name for the task.
      STACK_SIZE,    // Stack size in bytes, not words.
      NULL,          // Parameter passed into the task.
      5,             // Priority at which the task is created.
      xStack,        // Array to use as the task's stack.
      &xTaskBuffer); // Variable to hold the task's data structure.
}

#endif /* NO_MAIN */
