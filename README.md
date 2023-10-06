# ESP8266 Wifi Manager

Is a C esp-idf component for ESP8266 allows easy Wifi networking and http client.
Based on the Tony Pottier project https://github.com/tonyp7/esp32-wifi-manager

Advanced features:
  - support secure enterprise wifi networks
  - support flash logging
  - secure http client
  - OTA support

# Getting Started

The code is assembled and developed in a Docker container https://hub.docker.com/r/mbenabda/esp8266-rtos-sdk.

```bash 
docker pull mbenabda/esp8266-rtos-sdk
git clone https://github.com/vivask/esp8266-wifi-manager.git
cd esp8266-wifi-manager
docker run -dit --name wifi-manager -v $PWD:/project --privileged -v /dev:/dev -w /project mbenabda/esp8266-rtos-sdk
docker attach wifi-manager
make all flash monitor
```