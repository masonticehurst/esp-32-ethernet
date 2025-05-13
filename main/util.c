#include "lwip/ip4_addr.h"
#include "util.h"

#define STATIC_IP "10.0.0.37"
#define NETMASK "255.255.255.0"
#define GATEWAY "10.0.0.1"

void set_static_ip(esp_netif_t *netif) {
    esp_netif_ip_info_t ip_info;
    ip4addr_aton(STATIC_IP, (ip4_addr_t*)&ip_info.ip);
    ip4addr_aton(NETMASK, (ip4_addr_t*)&ip_info.netmask);
    ip4addr_aton(GATEWAY, (ip4_addr_t*)&ip_info.gw);

    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));
}