# ESP-32-Ethernet

## Overview

This project implements a dual-interface networking system on the ESP32-WROOM microcontroller using FreeRTOS and ESP-IDF. It prioritizes Ethernet connectivity via a W5500 module and will fall back to Wi-Fi using a shared static IP address when Ethernet connectivity is lost or becomes unreliable. Additionally, a lightweight HTTP web server runs as a FreeRTOS task, acting as a real-time heartbeat timer.

## Requirements

### Functional Requirements

* The system must have the ability to communicate to the W5500 over SPI
* The system must have the ability to transmit and receive Ethernet frames
* The system must fall over to Wi-Fi when Ethernet connectivity is lost
* The system must assign and reuse a persistent static IP across both interfaces
* The system must react and respond to network connectivity events
* The system must allow external devices to communicate over HTTP via a webserver
* The system must use task pinning to separate parallel tasks

### Non-Functional Requirements

* The transition between Ethernet and Wi-Fi should occur without any manual intervention
* Web server and networking interface connections should complete within 8 seconds of microcontroller boot
* The system must handle repeated disconnections and reconnections gracefully without crashing or requiring a reboot
