#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "wifi.h"
#include "util.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

static volatile bool eth_connected = false;

#include "esp_eth_driver.h"

/**
 * @brief Initialize Ethernet driver based on Espressif IoT Development Framework Configuration
 *
 * @param[out] eth_handle_out Initialized Ethernet driver handle
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_INVALID_ARG when passed invalid pointers
 *          - ESP_ERR_NO_MEM when there is no memory to allocate for Ethernet driver handles array
 *          - ESP_FAIL on any other failure
 */
esp_err_t eth_init(esp_eth_handle_t *eth_handle_out);

/**
 * @brief De-initialize array of Ethernet drivers
 * @note All Ethernet drivers in the array must be stopped prior calling this function.
 *
 * @param[in] eth_handle Ethernet driver to be de-initialized
 * @return
 *          - ESP_OK on success
 *          - ESP_ERR_INVALID_ARG when passed invalid pointers
 */
esp_err_t eth_deinit(esp_eth_handle_t *eth_handle);

/**
 * @brief Entry of the network FreeROTS task
 */
void network_task(void *arg);