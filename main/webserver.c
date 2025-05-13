#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "webserver.h"
#include "util.h"

static const char *TAG = "webserver";
static httpd_handle_t server = NULL;

/** HTTP GET handler to respond with "Hello World" */
esp_err_t hello_get_handler(httpd_req_t *req)
{
    char resp_buffer[64];
    uint64_t d = (uint64_t)(esp_timer_get_time() * 1E-6);
    sprintf(resp_buffer, "Up Time: %llu", d);
    httpd_resp_send(req, resp_buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/** URI handler structure for GET / */
httpd_uri_t hello = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = hello_get_handler,
    .user_ctx = NULL
};

/** Function to start the web server */
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &hello);
    }
    return server;
}

void webserver_task(void *arg) {
    // Delay to ensure Ethernet has time to initialize first
    vTaskDelay(pdMS_TO_TICKS(8000));

    if (server == NULL) {
        server = start_webserver();
        ESP_LOGI(TAG, "Webserver started on core %d", xPortGetCoreID());
    }

    vTaskDelete(NULL); // Webserver continues to run in background
}