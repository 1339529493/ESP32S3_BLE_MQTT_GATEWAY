#ifndef __OTA_H
#define __OTA_H

#include <inttypes.h>

void ota_verify_and_check_integrity();
void ota_start(uint8_t *url);
void manual_rollback(void);

#endif
