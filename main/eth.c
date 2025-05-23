#include "eth.h"

#define CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ 16
#define CONFIG_EXAMPLE_ETH_SPI_HOST 1

#define CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO 19
#define CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO 18
#define CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO 14
#define CONFIG_EXAMPLE_ETH_SPI_CS0_GPIO 25
#define CONFIG_EXAMPLE_ETH_SPI_INT0_GPIO -1
#define CONFIG_EXAMPLE_ETH_SPI_POLLING0_MS 10
#define CONFIG_EXAMPLE_ETH_SPI_PHY_RST0_GPIO -1
#define CONFIG_EXAMPLE_ETH_SPI_PHY_ADDR0 1

static const char *TAG = "eth_driver";

typedef struct
{
    uint8_t spi_cs_gpio;
    int8_t int_gpio;
    uint32_t polling_ms;
    int8_t phy_reset_gpio;
    uint8_t phy_addr;
    uint8_t *mac_addr;
} spi_eth_module_config_t;

static esp_err_t spi_bus_init(void)
{
    esp_err_t ret = ESP_OK;

    // Init SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
        .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
        .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_GOTO_ON_ERROR(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO),
                      err, TAG, "SPI host #%d init failed", CONFIG_EXAMPLE_ETH_SPI_HOST);

err:
    return ret;
}

static esp_eth_handle_t eth_init_spi(spi_eth_module_config_t *spi_eth_module_config, esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out)
{
    esp_eth_handle_t ret = NULL;

    // Init common MAC and PHY configs to default
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    // Update PHY config based on board specific configuration
    phy_config.phy_addr = spi_eth_module_config->phy_addr;
    phy_config.reset_gpio_num = spi_eth_module_config->phy_reset_gpio;

    // Configure SPI interface for specific SPI module
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .queue_size = 20,
        .spics_io_num = spi_eth_module_config->spi_cs_gpio};
    // Init vendor specific MAC config to default, and create new SPI Ethernet MAC instance
    // and new PHY instance based on board configuration
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(CONFIG_EXAMPLE_ETH_SPI_HOST, &spi_devcfg);
    w5500_config.int_gpio_num = spi_eth_module_config->int_gpio;
    w5500_config.poll_period_ms = spi_eth_module_config->polling_ms;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    // Init Ethernet driver to default and install it
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_GOTO_ON_FALSE(esp_eth_driver_install(&eth_config_spi, &eth_handle) == ESP_OK, NULL, err, TAG, "SPI Ethernet driver install failed");

    // The SPI Ethernet module might not have a burned factory MAC address, we can set it manually.
    if (spi_eth_module_config->mac_addr != NULL)
    {
        ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, spi_eth_module_config->mac_addr) == ESP_OK,
                          NULL, err, TAG, "SPI Ethernet MAC address config failed");
    }

    if (mac_out != NULL)
    {
        *mac_out = mac;
    }
    if (phy_out != NULL)
    {
        *phy_out = phy;
    }
    return eth_handle;

err:
    if (eth_handle != NULL)
    {
        esp_eth_driver_uninstall(eth_handle);
    }
    if (mac != NULL)
    {
        mac->del(mac);
    }
    if (phy != NULL)
    {
        phy->del(phy);
    }

    return ret;
}

esp_err_t eth_init(esp_eth_handle_t *eth_handle_out)
{
    esp_err_t ret = ESP_OK;
    esp_eth_handle_t *eth_handle = NULL;

    ESP_GOTO_ON_ERROR(spi_bus_init(), err, TAG, "SPI bus init failed");

    // Init specific SPI Ethernet module configuration from Kconfig (CS GPIO, Interrupt GPIO, etc.)
    spi_eth_module_config_t spi_eth_module_config;
    spi_eth_module_config.spi_cs_gpio = CONFIG_EXAMPLE_ETH_SPI_CS0_GPIO;
    spi_eth_module_config.int_gpio = CONFIG_EXAMPLE_ETH_SPI_INT0_GPIO;
    spi_eth_module_config.polling_ms = CONFIG_EXAMPLE_ETH_SPI_POLLING0_MS;
    spi_eth_module_config.phy_reset_gpio = CONFIG_EXAMPLE_ETH_SPI_PHY_RST0_GPIO;
    spi_eth_module_config.phy_addr = CONFIG_EXAMPLE_ETH_SPI_PHY_ADDR0;

    // The SPI Ethernet module(s) might not have a burned factory MAC address, hence use manually configured address(es).
    // In this example, Locally Administered MAC address derived from ESP32x base MAC address is used.
    // Note that Locally Administered OUI range should be used only when testing on a LAN under your control!
    uint8_t base_mac_addr[ETH_ADDR_LEN];
    ESP_GOTO_ON_ERROR(esp_efuse_mac_get_default(base_mac_addr), err, TAG, "get EFUSE MAC failed");
    uint8_t local_mac_1[ETH_ADDR_LEN];
    esp_derive_local_mac(local_mac_1, base_mac_addr);
    spi_eth_module_config.mac_addr = local_mac_1;

    eth_handle = eth_init_spi(&spi_eth_module_config, NULL, NULL);
    ESP_GOTO_ON_FALSE(eth_handle, ESP_FAIL, err, TAG, "SPI Ethernet init failed");

    *eth_handle_out = eth_handle;

err:
    return ret;
}

esp_err_t eth_deinit(esp_eth_handle_t *eth_handle)
{
    ESP_RETURN_ON_FALSE(eth_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Ethernet handle cannot be NULL");
    esp_eth_mac_t *mac = NULL;
    esp_eth_phy_t *phy = NULL;

    if (eth_handle != NULL)
    {
        esp_eth_get_mac_instance(eth_handle, &mac);
        esp_eth_get_phy_instance(eth_handle, &phy);
        ESP_RETURN_ON_ERROR(esp_eth_driver_uninstall(eth_handle), TAG, "Ethernet %p uninstall failed", eth_handle);
    }
    if (mac != NULL)
    {
        mac->del(mac);
    }
    if (phy != NULL)
    {
        phy->del(phy);
    }

    spi_bus_free(CONFIG_EXAMPLE_ETH_SPI_HOST);
    free(eth_handle);
    return ESP_OK;
}

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        eth_connected = true;
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        ESP_ERROR_CHECK(esp_wifi_stop());
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        eth_connected = false;
        ESP_LOGI(TAG, "Ethernet Link Down, Falling back to Wi-Fi...");
        wifi_connect();
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

void network_task(void *arg) {
    // Initialize Ethernet driver
    esp_eth_handle_t eth_handle;
    ESP_ERROR_CHECK(eth_init(&eth_handle));
    
    // Initialize TCP/IP network interface aka the esp-netif (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *eth_netif;
    esp_eth_netif_glue_handle_t eth_netif_glue;

    // Create instance(s) of esp-netif for Ethernet(s)
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&cfg);
    set_static_ip(eth_netif);
    eth_netif_glue = esp_eth_new_netif_glue(eth_handle);
    // Attach Ethernet driver to TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, eth_netif_glue));


    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    // Start Ethernet driver state machine
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    ESP_LOGI(TAG, "Ethernet drivers started on core %d", xPortGetCoreID());

    wifi_init();

    // Wait 5 seconds for Ethernet to come online
    vTaskDelay(pdMS_TO_TICKS(5000));

    if (!eth_connected) {
        wifi_connect();
    }

    vTaskDelete(NULL); // Kill task once done
}