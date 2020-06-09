#pragma once

#include <stdio.h>
#include "esp_log.h"
#include <cstring>
#include "esp_err.h"

class webhookIF {
public:
    webhookIF(const char *webhookURL, const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end);

    esp_err_t sendMessage(const char *content, int content_length);

private:
    static const char *TAG;
};