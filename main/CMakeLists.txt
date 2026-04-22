file(GLOB MQTT_SRCS "src/mqtts/*.c")
set(MQTT_INCLUDE "src/mqtts")

file(GLOB WIFI_SRCS "src/wifi/*.c")
set(WIFI_INCLUDE "src/wifi")

file(GLOB BLE_SRCS "src/ble/*.c")
set(BLE_INCLUDE "src/ble")

file(GLOB GATEWAY_SRCS "src/gateway/*.c")
set(GATEWAY_INCLUDE "src/gateway")

file(GLOB UTIL_SRCS "src/util/*.c")
set(UTIL_INCLUDE "src/util")

set(GATEWAY_SRCS ${MQTT_SRCS} ${WIFI_SRCS} ${BLE_SRCS} ${GATEWAY_SRCS} ${UTIL_SRCS})
set(GATEWAY_INCLUDE ${MQTT_INCLUDE} ${WIFI_INCLUDE} ${BLE_INCLUDE} ${GATEWAY_INCLUDE} ${UTIL_INCLUDE})

idf_component_register(SRCS "main.c" ${GATEWAY_SRCS}
                    REQUIRES bt nvs_flash mqtt
                    INCLUDE_DIRS "." ${GATEWAY_INCLUDE})