idf_component_register(
    SRCS "util.c" "wifi.c" "webserver.c" "eth.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_netif esp_wifi esp_eth esp_timer esp_system esp_driver_gpio nvs_flash esp_http_server spi_flash)
