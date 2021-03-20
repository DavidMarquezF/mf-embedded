#include "mf_temp.h"
#include "mf_log.h"

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"


#define GPIO_DS18B20_0 (CONFIG_ONE_WIRE_GPIO)
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD (1000) // milliseconds

static  DS18B20_Info *tempDevice;
static  OneWireBus *owb;
static owb_rmt_driver_info rmt_driver_info;

// Code from https://github.com/DavidAntliff/esp32-ds18b20-example/blob/master/main/app_main.c
uint8_t mf_temp_setup(void)
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
    return 0;
}


uint8_t mf_temp_get_value(float* result)
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

uint8_t mf_temp_destroy(void){
      ds18b20_free(&tempDevice);
    
    owb_uninitialize(owb);
    return 0;

}