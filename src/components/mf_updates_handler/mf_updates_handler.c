#include "mf_updates_handler.h"

#include "oc_swupdate.h"
#include "messaging/coap/coap.h"
#include "messaging/coap/transactions.h"
#include "oc_client_state.h"
#include "oc_blockwise.h"
#include "oc_api.h"

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
    (void)url;
    oc_swupdate_notify_downloaded(device, "2.0", OC_SWUPDATE_RESULT_SUCCESS);
    return 0;
}

static int perform_upgrade(size_t device, const char *url)
{
    (void)url;
    oc_swupdate_notify_upgrading(device, "2.0", oc_clock_time(),
                                 OC_SWUPDATE_RESULT_SUCCESS);

    oc_swupdate_notify_done(device, OC_SWUPDATE_RESULT_SUCCESS);
    return 0;
}

oc_event_callback_retval_t oc_ri_remove_client_cb(void *data);


static coap_transaction_t *transaction;
static coap_packet_t request[1];
static oc_client_cb_t *client_cb;
#ifdef OC_BLOCK_WISE
static oc_blockwise_state_t *response_buffer = NULL;
#endif /* OC_BLOCK_WISE */

static oc_endpoint_t *endpoint;

static bool handle_block(oc_blockwise_state_t *buffer,
                          uint32_t incoming_block_offset,
                          const uint8_t *incoming_block,
                          uint32_t incoming_block_size){
                            (void)buffer;
                            (void)incoming_block_offset;
  PRINT("%.*s",incoming_block_size, incoming_block);
  return true;
}

static bool
prepare_coap_request(oc_client_cb_t *cb)
{
  coap_message_type_t type = COAP_TYPE_NON;

  if (cb->qos == HIGH_QOS) {
    type = COAP_TYPE_CON;
  }

  transaction = coap_new_transaction(cb->mid, &cb->endpoint);

  if (!transaction) {
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
  coap_set_header_block2(request, 0, 0, 1024);


  coap_set_token(request, cb->token, cb->token_len);

  coap_set_header_uri_path(request, oc_string(cb->uri), oc_string_len(cb->uri));

  if (oc_string_len(cb->query) > 0) {
    coap_set_header_uri_query(request, oc_string(cb->query));
  }

  client_cb = cb;

  response_buffer = oc_blockwise_alloc_response_buffer(
            oc_string(client_cb->uri) + 1, oc_string_len(client_cb->uri) - 1,
            endpoint, client_cb->method, OC_BLOCKWISE_CLIENT);
  if (response_buffer) {
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
  if (transaction->message->length > 0) {
    coap_send_transaction(transaction);

    if (client_cb->observe_seq == -1) {
      if (client_cb->qos == LOW_QOS)
        oc_set_delayed_callback(client_cb, &oc_ri_remove_client_cb,
                                OC_NON_LIFETIME);
      else
        oc_set_delayed_callback(client_cb, &oc_ri_remove_client_cb,
                                OC_EXCHANGE_LIFETIME);
    }

    success = true;
  } else {
    coap_clear_transaction(transaction);
    oc_ri_remove_client_cb(client_cb);
  }

  transaction = NULL;
  client_cb = NULL;

  return success;
}

static void finish_download_handler(oc_client_response_t * r){
  (void)r;
 // PRINT("%s", r->_payload);
    //TODO: Clear response buffer, maybe oc_blockwise_scrub_buffers_for_client_cb?

}

void download_update_coap()
{
  PRINT("\n\n\nDOWNLOADING\n");
  endpoint = oc_new_endpoint();
  oc_string_t enpoint_string;
  char enp_string[] = "coap://192.168.1.120";
  oc_new_string(&enpoint_string, enp_string, strlen(enp_string));

  oc_string_t uri_string;
  char uri[] = "/a"; //TODO: Extract this from the purl set in the update resource
  oc_new_string(&uri_string, uri, strlen(uri));

  oc_string_to_endpoint(&enpoint_string, endpoint, NULL);
  

  oc_client_handler_t client_handler;
  client_handler.response = finish_download_handler;

  oc_client_cb_t *cb = oc_ri_alloc_client_cb_custom_accept(uri, endpoint, OC_GET, NULL,
                                             client_handler, HIGH_QOS, NULL, TEXT_PLAIN);
  if(!cb){
    PRINT("Error creating CB\n");
    return;
  }
  bool status = false;
  status = prepare_coap_request(cb);
  if(status){
    status = dispatch_coap_request();
  }
  else{
    PRINT("Error preparing CoAP\n");
    return;
  }
  if(!status){
    PRINT("Error dispatching CoAp\n");
  }
  // oc_free_endpoint(endpoint);

}




void mf_updates_handler_init(void)
{
    static oc_swupdate_cb_t swupdate_impl;
    swupdate_impl.validate_purl = validate_purl;
    swupdate_impl.check_new_version = check_new_version;
    swupdate_impl.download_update = download_update;
    swupdate_impl.perform_upgrade = perform_upgrade;
    oc_swupdate_set_impl(&swupdate_impl);
}