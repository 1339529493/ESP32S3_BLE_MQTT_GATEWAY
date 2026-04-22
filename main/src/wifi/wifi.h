#ifndef __WIFI_H
#define __WIFI_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"

extern const char *WIFI_TAG;

void print_auth_mode(int authmode);
void print_cipher_type(int pairwise_cipher, int group_cipher);
void connet_display(uint8_t flag);

void wifi_sta_init(void);

#endif
