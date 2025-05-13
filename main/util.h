#include "esp_netif.h"

/**
 * @brief Disable DHCP and set static IP address
 *
 * @param[in] eth_handle Network interface
 * @return
 *          - ESP_OK on success
 */
void set_static_ip(esp_netif_t *netif);