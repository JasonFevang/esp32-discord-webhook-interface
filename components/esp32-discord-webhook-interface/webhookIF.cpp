#include "webhookIF.hpp"

const char * webhookIF::TAG = "webhookIF";

esp_err_t webhookIF::setup(const char *webhookURL, 
    const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end){
    ESP_LOGI(TAG, "setup");

    int size = binary_server_root_cert_end - binary_server_root_cert_begin;

    esp_tls_cfg_t cfg = { 0 };
    cfg.cacert_pem_buf  = binary_server_root_cert_begin;
    cfg.cacert_pem_bytes = static_cast<unsigned int>(binary_server_root_cert_end - binary_server_root_cert_begin);

    m_tls = esp_tls_conn_http_new(webhookURL, &cfg);

    m_webhook_url = webhookURL;

    if(m_tls == nullptr) {
        ESP_LOGE(TAG, "Connection failed...");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t webhookIF::sendMessage(const char *content, int content_length){
    ESP_LOGI(TAG, "sendMessage");

    // convert content_length to a char array
    char len[12];
    sprintf(len, "%d", content_length + 14);

    ESP_LOGI(TAG, "len %d", strlen(len));
    ESP_LOGI(TAG, "wh %d", strlen(m_webhook_url));

    int message_alloc_len = 154 + strlen(m_webhook_url)+strlen(len)+content_length + 200;
    ESP_LOGI(TAG, "Message alloc length %d", message_alloc_len);

    // construct message
    char *rqst = new char[message_alloc_len]; // remove the 200, just for easier development
    strcpy(rqst, "POST "); //5
    strcat(rqst, m_webhook_url); // strlen(m_webhook_url)
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

    size_t written_bytes = 0;
    int ret;
    printf("%s", rqst);

    // Write http message to discord
    do {
        ret = esp_tls_conn_write(m_tls, 
                                    rqst + written_bytes, 
                                    strlen(rqst) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x\n", ret);
            return ESP_FAIL;
        }
    } while(written_bytes < strlen(rqst));

    // Read http response

    do
    {
        // buffer to read the http response into
        char buf[512];

        int len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(m_tls, (char *)buf, len);
        
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