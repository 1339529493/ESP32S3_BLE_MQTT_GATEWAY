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

file(GLOB UI_DESKTOP_SRCS "src/display/desktop/*.c" "src/display/desktop/custom/*.c" "src/display/desktop/generated/*.c" "src/display/desktop/generated/guider_fonts/*.c" "src/display/desktop/generated/images/*.c")
set(UI_DESKTOP_INCLUDE "src/util" "src/display/desktop/custom" "src/display/desktop/custom" "src/display/desktop/generated")

file(GLOB UI_MENU_SRCS "src/display/menu/*.c")
set(UI_MENU_INCLUDE "src/util" "src/display")

file(GLOB UI_SRCS "src/display/*.c" "src/display/smf/*.c" ${UI_DESKTOP_SRCS} ${UI_MENU_SRCS})
set(UI_INCLUDE "src/display" "src/display/smf" ${UI_DESKTOP_INCLUDE} ${UI_MENU_INCLUDE})

set(GATEWAY_SRCS ${MQTT_SRCS} ${WIFI_SRCS} ${BLE_SRCS} ${KEY_SRCS} ${GATEWAY_SRCS} ${UTIL_SRCS} ${UI_SRCS})
set(GATEWAY_INCLUDE ${MQTT_INCLUDE} ${WIFI_INCLUDE} ${BLE_INCLUDE} ${KEY_INCLUDE} ${GATEWAY_INCLUDE} ${UTIL_INCLUDE} ${UI_INCLUDE})

idf_component_register(SRCS "main.c" ${GATEWAY_SRCS}
                    REQUIRES bt nvs_flash mqtt esp_http_client BSP
                    INCLUDE_DIRS "." ${GATEWAY_INCLUDE})