#include "esp_timer.h"
#include "esp_http_server.h"

/**
 * @brief Configures pages of webserver and starts service
 *
 * @return
 *          - HTTPD Handle
 */
httpd_handle_t start_webserver(void);

/**
 * @brief Entry of the webserver FreeROTS task
 */
void webserver_task(void *arg);