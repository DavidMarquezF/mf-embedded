#include "mf_updates_handler.h"

#include "oc_swupdate.h"
#include "messaging/coap/coap.h"
#include "messaging/coap/transactions.h"
#include "oc_client_state.h"
#include "oc_blockwise.h"
#include "oc_api.h"

#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "native_ota_example";
static int binary_file_length;
/*deal with all receive packet*/
static bool image_header_was_checked;
static const esp_partition_t *configured_partition;
static const esp_partition_t *running_partition;
static const esp_partition_t *update_partition;
static esp_ota_handle_t update_handle;
static size_t device_upgrading;

static void prepare_update(void)
{
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    update_handle = 0;
    update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA example");

    configured_partition = esp_ota_get_boot_partition();
    running_partition = esp_ota_get_running_partition();

    if (configured_partition != running_partition)
    {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured_partition->address, running_partition->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running_partition->type, running_partition->subtype, running_partition->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    binary_file_length = 0;
    image_header_was_checked = false;
}

oc_event_callback_retval_t oc_ri_remove_client_cb(void *data);

static coap_transaction_t *transaction;
static coap_packet_t request[1];
static oc_client_cb_t *client_cb;
#ifdef OC_BLOCK_WISE
static oc_blockwise_state_t *response_buffer = NULL;
#endif /* OC_BLOCK_WISE */

static oc_endpoint_t *endpoint;


static bool handle_block_intern(oc_blockwise_state_t *buffer,
                         uint32_t incoming_block_offset,
                         const uint8_t *incoming_block,
                         uint32_t incoming_block_size)
{
    (void)buffer;
    (void)incoming_block_offset;
    
  esp_err_t err;

    if (incoming_block_size <= 0)
        return true;
    if (image_header_was_checked == false)
    {
        ESP_LOGI(TAG, "Checking header");
        esp_app_desc_t new_app_info;
        if (incoming_block_size > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
        {
            // check current version with downloading
            memcpy(&new_app_info, &incoming_block[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

            esp_app_desc_t running_app_info;
            if (esp_ota_get_partition_description(running_partition, &running_app_info) == ESP_OK)
            {
                ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
            }

            const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
            esp_app_desc_t invalid_app_info;
            if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
            {
                ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
            }

            // check current version with last invalid partition
            if (last_invalid_app != NULL)
            {
                if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0)
                {
                    ESP_LOGW(TAG, "New version is the same as invalid version.");
                    ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                    ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                    return false;
                }
            }
            //#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
            if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0)
            {
                ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                return false;
            }
            //#endif

            image_header_was_checked = true;

            err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                esp_ota_abort(update_handle);
                return false;
            }
            ESP_LOGI(TAG, "esp_ota_begin succeeded");
        }
        else
        {
            //TODO: If the blocks are less than the size specified in the if it will abourt the update.
            // Blocks can be as small as 8 bytes so instead of just aborting we should use a buffer to store
            // the block until it was big enough. Maybe we can use the already created blockwise buffer
            // This only would apply to the first segment (we need to verify versions, etc.). From there it doesn't matter
            ESP_LOGE(TAG, "received package is not fit len");
            esp_ota_abort(update_handle);
            return false;
        }
    }
    
    ESP_LOGI(TAG, "Writing block to OTA");  
    err = esp_ota_write(update_handle, (const void *)incoming_block, incoming_block_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "error writing to OTA");
        esp_ota_abort(update_handle);
        return false;
    }
    binary_file_length += incoming_block_size;
    ESP_LOGD(TAG, "Written image length %d", binary_file_length);

    return true;
}


static bool handle_block(oc_blockwise_state_t *buffer,
                         uint32_t incoming_block_offset,
                         const uint8_t *incoming_block,
                         uint32_t incoming_block_size)
{   
    PRINT("Received block, %d %d\n", incoming_block_offset, incoming_block_size);
    bool result = handle_block_intern(buffer, incoming_block_offset, incoming_block, incoming_block_size);
    return result;
}

static bool
prepare_coap_request(oc_client_cb_t *cb)
{
    coap_message_type_t type = COAP_TYPE_NON;

    if (cb->qos == HIGH_QOS)
    {
        type = COAP_TYPE_CON;
    }

    transaction = coap_new_transaction(cb->mid, &cb->endpoint);

    if (!transaction)
    {
        return false;
    }

    // oc_rep_new(transaction->message->data + COAP_MAX_HEADER_SIZE, OC_BLOCK_SIZE);

    //#ifdef OC_TCP
    // if (cb->endpoint.flags & TCP) {
    //   coap_tcp_init_message(request, cb->method);
    // } else
    //#endif /* OC_TCP */
    {
        coap_udp_init_message(request, type, cb->method, cb->mid);
    }

    coap_set_header_accept(request, TEXT_PLAIN);
    //coap_set_header_block2(request, 0, 0, 2048);

    coap_set_token(request, cb->token, cb->token_len);

    coap_set_header_uri_path(request, oc_string(cb->uri), oc_string_len(cb->uri));

    if (oc_string_len(cb->query) > 0)
    {
        coap_set_header_uri_query(request, oc_string(cb->query));
    }

    client_cb = cb;

    response_buffer = oc_blockwise_alloc_response_buffer(
        oc_string(client_cb->uri) + 1, oc_string_len(client_cb->uri) - 1,
        endpoint, client_cb->method, OC_BLOCKWISE_CLIENT);
    if (response_buffer)
    {
        response_buffer->client_cb = client_cb;
        response_buffer->block_handle_cb = handle_block;
    }
    else
        return false;

    return true;
}

static bool
dispatch_coap_request(void)
{

    bool success = false;
    transaction->message->length =
        coap_serialize_message(request, transaction->message->data);
    if (transaction->message->length > 0)
    {
        coap_send_transaction(transaction);

        if (client_cb->observe_seq == -1)
        {
            if (client_cb->qos == LOW_QOS)
                oc_set_delayed_callback(client_cb, &oc_ri_remove_client_cb,
                                        OC_NON_LIFETIME);
            else
                oc_set_delayed_callback(client_cb, &oc_ri_remove_client_cb,
                                        OC_EXCHANGE_LIFETIME);
        }

        success = true;
    }
    else
    {
        coap_clear_transaction(transaction);
        oc_ri_remove_client_cb(client_cb);
    }

    transaction = NULL;
    client_cb = NULL;

    return success;
}

static void finish_download_handler(oc_client_response_t *r)
{
    if(r->code != OC_STATUS_OK){
        if(r->code == OC_STATUS_SERVICE_UNAVAILABLE){
            ESP_LOGW(TAG, "Couldn't fully download, service unavailable");
        }
        
        oc_swupdate_notify_downloaded(device_upgrading, "2.0", OC_SWUPDATE_RESULT_CONN_FAIL);
        esp_ota_abort(update_handle);
        return;
    }
    PRINT("Code: %d\n", r->code);
    // PRINT("%s", r->_payload);
    //TODO: Clear response buffer, maybe oc_blockwise_scrub_buffers_for_client_cb?
    // TODO: is it called if it was not successfull?
    PRINT("Perform upgrade");
    esp_err_t err;
    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        return;
    }
    oc_swupdate_notify_downloaded(device_upgrading, "2.0", OC_SWUPDATE_RESULT_SUCCESS);  
}

void download_update_coap(void)
{

}


static int validate_purl(const char *purl)
{
    /*if (instance.is_valid() == 0)
    {
        return -1;
    }*/
    return 0;
}

static int check_new_version(size_t device, const char *url, const char *version)
{
    if (!url)
    {
        oc_swupdate_notify_done(device, OC_SWUPDATE_RESULT_INVALID_URL);
        return -1;
    }
    PRINT("Package url %s\n", url);

    if (version)
    {
        PRINT("Package version: %s\n", version);
    }
    oc_swupdate_notify_new_version_available(device, "2.0",
                                             OC_SWUPDATE_RESULT_SUCCESS);
    return 0;
}

static int download_update(size_t device, const char *url)
{
    if(!url)
        return -1;
    device_upgrading=device;
    PRINT("\n\n\nDOWNLOADING\n");
    prepare_update();
    endpoint = oc_new_endpoint();
    oc_string_t enpoint_string;
    //if(!url){
     //   char enp_string[] = "coap://192.168.1.120/a";
      //  oc_new_string(&enpoint_string, enp_string, strlen(enp_string));    
    //}

    oc_new_string(&enpoint_string, url, strlen(url));
    oc_string_t uri_string;
    oc_string_to_endpoint(&enpoint_string, endpoint, &uri_string);
    oc_free_string(&enpoint_string);

    oc_client_handler_t client_handler;
    client_handler.response = finish_download_handler;

    oc_client_cb_t *cb = oc_ri_alloc_client_cb_custom_accept(oc_string(uri_string), endpoint, OC_GET, NULL,
                                                             client_handler, HIGH_QOS, NULL, TEXT_PLAIN);
    oc_free_string(&uri_string);


    if (!cb)
    {
        PRINT("Error creating CB\n");
        oc_swupdate_notify_downloaded(device, "2.0", OC_SWUPDATE_RESULT_LESS_RAM);
        return -1;
    }

    if (prepare_coap_request(cb))
    {
        if(!dispatch_coap_request()){
            PRINT("Error dispatching CoAp\n");
            oc_swupdate_notify_downloaded(device, "2.0", OC_SWUPDATE_RESULT_LESS_RAM);
            return -1;
        }
    }
    else{
        PRINT("Error preparing CoAP\n");
        oc_swupdate_notify_downloaded(device, "2.0", OC_SWUPDATE_RESULT_LESS_RAM);
        return -1;
    }
        
    // oc_free_endpoint(endpoint);
    return 0;
}

static int perform_upgrade(size_t device, const char *url)
{
    (void)url;       
    esp_err_t err;


    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        oc_swupdate_notify_upgrading(device, "2.0", oc_clock_time(),
                                 OC_SWUPDATE_RESULT_UPGRADE_FAIL);
        return -1;
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
    oc_swupdate_notify_upgrading(device, "2.0", oc_clock_time(),
                                 OC_SWUPDATE_RESULT_SUCCESS);
    esp_restart();

    return 0;
}


void mf_updates_handler_init_check_if_updated(void){
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
           // bool diagnostic_is_ok = diagnostic();
            //if (diagnostic_is_ok) {
             //   ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                      //         oc_swupdate_notify_done((size_t)0, OC_SWUPDATE_RESULT_SUCCESS);

                esp_ota_mark_app_valid_cancel_rollback();
                // TODO: save the device being upgraded somewhere

            //} else {
             //   ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
              //  esp_ota_mark_app_invalid_rollback_and_reboot();
           // }
        }
    }
}


void mf_updates_handler_init(void)
{
    PRINT("Enable updates!");
    static oc_swupdate_cb_t swupdate_impl;
    swupdate_impl.validate_purl = validate_purl;
    swupdate_impl.check_new_version = check_new_version;
    swupdate_impl.download_update = download_update;
    swupdate_impl.perform_upgrade = perform_upgrade;
    oc_swupdate_set_impl(&swupdate_impl);
}