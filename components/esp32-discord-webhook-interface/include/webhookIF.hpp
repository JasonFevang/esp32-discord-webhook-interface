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
    /**
     * @brief Construct a webhook interface object. One object can interact with one webhook
     * To use multiple webhooks, instantiate multiple instances of this class
     * @param webhookURL character array containing the webhook url
     * This is always copied by reference to save memory, do not invalidate this pointer
     * @param binary_server_root_cert_begin pointer to the beginning of the discord root certificate. 
     * This is always copied by reference to save memory, do not invalidate this pointer
     * Intended to be loaded onto the esp32 using EMBED_TXTFILES CMake directive
     */
    webhookIF(const char *webhook_URI, const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end);

    esp_err_t send_message(const char *content, int content_length);

    // For debugging
    esp_err_t send_message_print_response(const char *content, int content_length);

private:
    static const char *TAG;
    const char *m_webhook_uri;
    esp_tls_cfg_t m_cfg = { 0 };

    const int m_response_buf_size = 512;

    esp_tls_t *m_tls;

    esp_err_t start_connection();
    esp_err_t write_request(const char *content, int content_length);
    esp_err_t print_response();
    void close_connection();
};