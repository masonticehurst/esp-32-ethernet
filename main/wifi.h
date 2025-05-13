#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "util.h"

/**
 * @brief Initialize WiFi drivers and parameters
 * Only called once during main() initialization
 */
void wifi_init(void);

/**
 * @brief Attempts to connect to WiFi access point
 */
void wifi_connect(void);