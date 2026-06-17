file(GLOB MQTT_SRCS "src/mqtts/*.c")
set(MQTT_INCLUDE "src/mqtts")

file(GLOB WIFI_SRCS "src/wifi/*.c")
set(WIFI_INCLUDE "src/wifi")

file(GLOB BLE_SRCS "src/ble/*.c")
set(BLE_INCLUDE "src/ble")

file(GLOB KEY_SRCS "src/key/*.c")
set(KEY_INCLUDE "src/key")

file(GLOB GATEWAY_SRCS "src/gateway/*.c")
set(GATEWAY_INCLUDE "src/gateway")

file(GLOB UTIL_SRCS "src/util/*.c")
set(UTIL_INCLUDE "src/util")

file(GLOB UI_DESKTOP_SRCS "src/display/*.c" "src/display/custom/*.c" "src/display/generated/*.c" "src/display/generated/guider_fonts/*.c" "src/display/generated/images/*.c")
set(UI_DESKTOP_INCLUDE "src/util" "src/display/custom" "src/display/custom" "src/display/generated")

file(GLOB UI_SRCS "src/display/*.c" "src/display/smf/*.c" ${UI_DESKTOP_SRCS} ${UI_MENU_SRCS})
set(UI_INCLUDE "src/display" "src/display/smf" ${UI_DESKTOP_INCLUDE} ${UI_MENU_INCLUDE})

file(GLOB OTA_SRCS "src/ota/*.c")
set(OTA_INCLUDE "src/ota")

set(GATEWAY_SRCS ${MQTT_SRCS} ${WIFI_SRCS} ${BLE_SRCS} ${KEY_SRCS} ${GATEWAY_SRCS} ${UTIL_SRCS} ${UI_SRCS} ${OTA_SRCS})
set(GATEWAY_INCLUDE ${MQTT_INCLUDE} ${WIFI_INCLUDE} ${BLE_INCLUDE} ${KEY_INCLUDE} ${GATEWAY_INCLUDE} ${UTIL_INCLUDE} ${UI_INCLUDE} ${OTA_INCLUDE})

idf_component_register(SRCS "main.c" ${GATEWAY_SRCS}
                    REQUIRES bt nvs_flash mqtt BSP json esp_http_client app_update
                    INCLUDE_DIRS "." ${GATEWAY_INCLUDE})