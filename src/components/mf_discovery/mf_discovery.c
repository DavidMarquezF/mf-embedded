#include "mf_discovery.h"
#include "mf_discovery_spi.h"
#include "mf_discovery_i2c.h"
#include "oc_api.h"

#define MF_RSRVD_RES_TYPE_DISCOVERY "mf.r.discovery"
#define MF_RSRVD_URI_DISCOVERY "/mfDiscover"
#define MF_DISCOVERY_MAX_DEVICES 10

int mf_discovery_discover(mf_device_t *dev, uint8_t max_devices)
{
    mf_discovery_spi_init_discovery();
    int i = 0;
    // Discovers new spi devices. If it doesn't find any more devices it will stop
    while (i < max_devices && mf_discovery_spi_discover_next(&dev[i]) == 0 && dev[i] != MF_DEVICE_INVALID)
        i++;
    
    mf_discovery_i2c_init_discovery();
    while (i < max_devices && mf_discovery_i2c_discover_next(&dev[i]) == 0 && dev[i] != MF_DEVICE_INVALID)
        i++;
    

    return i;
}

static void
get_discovery(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
    PRINT("get discovery\n");
    mf_device_t devices[MF_DISCOVERY_MAX_DEVICES];
    int discovered = mf_discovery_discover(devices, MF_DISCOVERY_MAX_DEVICES);
    oc_rep_start_root_object();
    switch (iface_mask)
    {
    case OC_IF_BASELINE:
        oc_process_baseline_interface(request->resource);
    /* fall through */
    case OC_IF_R:
        oc_rep_set_int_array(root, devices, devices, discovered);
        break;
    default:
        break;
    }
    oc_rep_end_root_object();
    oc_send_response(request, OC_STATUS_OK);
}

uint8_t mf_discovery_register_resource(size_t device)
{
    oc_resource_t *res = oc_new_resource(NULL, MF_RSRVD_URI_DISCOVERY, 1, device);
    oc_resource_bind_resource_type(res, MF_RSRVD_RES_TYPE_DISCOVERY);
    oc_resource_bind_resource_interface(res, OC_IF_BASELINE);
    oc_resource_bind_resource_interface(res, OC_IF_R);
    oc_resource_set_default_interface(res, OC_IF_R);
    oc_resource_set_request_handler(res, OC_GET, get_discovery, NULL);
    oc_resource_set_discoverable(res, true);
#ifdef OC_CLOUD
    oc_cloud_add_resource(res);
#endif

    oc_add_resource(res);
    return 0;
}