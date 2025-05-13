#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "util.h"
#include "eth.h"
#include "wifi.h"
#include "webserver.h"

#define CORE_0              0
#define CORE_1              1

#define WEBSERVER_PRIORITY  1
#define WEB_TASK_STACK_SIZE 4096

#define DRIVER_PRIORITY     2
#define ETH_TASK_STACK_SIZE 8192

void app_main(void)
{
    xTaskCreatePinnedToCore(
        network_task,
        "network_task",
        ETH_TASK_STACK_SIZE,
        NULL,
        DRIVER_PRIORITY,
        NULL,
        CORE_0             // Core 0
    );

    xTaskCreatePinnedToCore(
        webserver_task,
        "webserver_task",
        WEB_TASK_STACK_SIZE,
        NULL,
        WEBSERVER_PRIORITY,
        NULL,
        CORE_1              // Core 1
    );
}
