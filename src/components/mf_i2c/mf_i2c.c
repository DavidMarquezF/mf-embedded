#include "mf_i2c.h"
#include "driver/i2c.h"


#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO 27               /* gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 14               /* gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(0)
#define I2C_MASTER_FREQ_HZ 10 * 1000        /* I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /* I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /* I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE              /* I2C master write */
#define READ_BIT I2C_MASTER_READ                /* I2C master read */
#define ACK_CHECK_EN 0x1                        /* I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /* I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /* I2C ack value */
#define NACK_VAL 0x1                            /* I2C nack value */

uint8_t mf_i2c_init(void){
 int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return 1;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0) == ESP_OK ? 0 : 1;
}

static uint8_t mf_i2c_send_internal(mf_i2c_device_t device, uint8_t* message, size_t length){
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
     i2c_master_start(cmd);
     i2c_master_write_byte(cmd, (device << 1) | WRITE_BIT, ACK_CHECK_EN);
    if(message != NULL)
        i2c_master_write(cmd, message,length, ACK_CHECK_EN);
     i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK ? 0 : 1;
}

static uint8_t mf_i2c_receive_internal(mf_i2c_device_t device, void* receive_buffer, size_t length){
 int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, device << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, receive_buffer, length, ACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK ? 0 : 1;
}

uint8_t mf_i2c_send_message(mf_i2c_device_t device, uint8_t* message, size_t length){
    return mf_i2c_send_internal(device, message, length);
}

uint8_t mf_i2c_receive_message(mf_i2c_device_t device, void* receive_buffer, size_t length){
    return mf_i2c_receive_internal(device, receive_buffer, length);
}

uint8_t mf_i2c_send_and_receive_message(mf_i2c_device_t device, uint8_t *message, size_t message_size, void* receive_buffer, size_t receive_size){
    
    int ret = mf_i2c_send_internal(device, message, message_size);
    if(ret != 0)
        return ret;
    //vTaskDelay(1000 / portTICK_RATE_MS);
    return mf_i2c_receive_internal(device, receive_buffer, receive_size);
}
