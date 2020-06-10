#pragma once

#include <stdio.h>
#include "esp_log.h"
#include <cstring>
#include "esp_err.h"

#include "esp_tls.h"
/**
 * @brief Provides a simple interface to send messages to a discord via a webhook
 * Requires the webhook url, and the discord root certificate
 */
class webhookIF {
public:
    webhookIF(const char *webhookURL, const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end);

    esp_err_t sendMessage(const char *content, int content_length);

private:
    static const char *TAG;
    char m_webhook_uri[128]; // Not sure if this will ever be too short
    const uint8_t *m_root_cert;
    unsigned int m_root_cert_len;

    const int m_response_buf_size = 512;
};