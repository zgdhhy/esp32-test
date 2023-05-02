#ifndef MYOTA_H
#define MYOTA_H
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"

typedef void (*ota_progress_cb)(int,int);

void ota_upgrade(char* url);
void ota_upgrade_progress(char* url,ota_progress_cb ota_progress_handler);
#endif