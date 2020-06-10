#include "webhookIF.hpp"

const char * webhookIF::TAG = "webhookIF";

webhookIF::webhookIF(const char *webhook_URI, 
    const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end){

    m_root_cert = binary_server_root_cert_begin;
    m_root_cert_len = binary_server_root_cert_end - binary_server_root_cert_begin;

    if(strlen(webhook_URI) > sizeof(m_webhook_uri)){
        ESP_LOGE(TAG, "Unexpectedly long webhook uri");
        return;
    }
    strcpy(m_webhook_uri, webhook_URI);
}

esp_err_t webhookIF::sendMessage(const char *content, int content_length){
    ESP_LOGI(TAG, "sendMessage \"%s\"", content);

    // Start connection
    esp_tls_cfg_t cfg = { 0 };
    cfg.cacert_pem_buf  = m_root_cert;
    cfg.cacert_pem_bytes = m_root_cert_len;

    esp_tls_t *tls = esp_tls_conn_http_new(m_webhook_uri, &cfg);

    if(tls == nullptr) {
        ESP_LOGE(TAG, "Connection failed");
        return ESP_ERR_ESP_TLS_FAILED_CONNECT_TO_HOST;
    }

    ESP_LOGI(TAG, "tls %d", tls->sockfd);

    // convert content_length to a char array
    char len[12];
    sprintf(len, "%d", content_length + 14);

    //ESP_LOGI(TAG, "len %d", strlen(len));
    //ESP_LOGI(TAG, "wh %d", strlen(m_webhook_uri));

    int message_alloc_len = 154 + strlen(m_webhook_uri)+strlen(len)+content_length + 200;
    //ESP_LOGI(TAG, "Message alloc length %d", message_alloc_len);

    // construct message
    char *rqst = new char[message_alloc_len]; // remove the 200, just for easier development
    strcpy(rqst, "POST "); //5
    strcat(rqst, m_webhook_uri); // strlen(m_webhook_url)
    strcat(rqst, " HTTP/1.0\r\n"); //11
    strcat(rqst, "User-Agent: esp-idf/1.0 esp32\r\n"); //31
    //general header
    strcat(rqst, "Connection: close\r\n" ); //19
    //request header
    strcat(rqst, "Host: discord.com\r\n"); //23
    //entity headers
    strcat(rqst, "Content-Type: application/json\r\n"); //32
    strcat(rqst, "Content-Length:"); //15
    strcat(rqst, len); // strlen(len) lol length of the length
    strcat(rqst, "\r\n"); //2
    //Newline break
    strcat(rqst, "\r\n"); //2
    // content in json
    strcat(rqst, "{\"content\":\""); //12
    strcat(rqst, content); //content_length
    strcat(rqst, "\"}"); //2
    printf("%s", rqst);
    ESP_LOGI(TAG, "rqst len %d", strlen(rqst));

    size_t written_bytes = 0;
    int ret = 0;

    // Write https message to discord
    do {
        ret = esp_tls_conn_write(tls, 
                                    rqst + written_bytes, 
                                    strlen(rqst) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x\n", ret);
            free(rqst);
            return ESP_FAIL;
        }
    } while(written_bytes < strlen(rqst));

    // Done with the request now, free that memory
    free(rqst);

    // Read http response
    do
    {
        // buffer to read the http response into
        char buf[m_response_buf_size] = { 0 };

        int len = sizeof(buf) - 1;
        ESP_LOGI(TAG, "buf len %d", len);
        ret = esp_tls_conn_read(tls, (char *)buf, len);
        
        if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
            continue;
        
        if(ret < 0)
        {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
            return ESP_FAIL;
        }

        if(ret == 0)
        {
            ESP_LOGI(TAG, "connection closed");
            return ESP_OK;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);
        /* Print response directly to stdout as it is read */
        for(int i = 0; i < len; i++) {
            putchar(buf[i]);
        }
    } while(1);
}