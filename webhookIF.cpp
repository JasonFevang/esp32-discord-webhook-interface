#include "webhookIF.hpp"

const char * webhookIF::TAG = "webhookIF";

webhookIF::webhookIF(const char *webhook_URI, 
    const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end){

    m_cfg.cacert_pem_buf  = binary_server_root_cert_begin;
    m_cfg.cacert_pem_bytes = binary_server_root_cert_end - binary_server_root_cert_begin;
    m_webhook_uri = webhook_URI;
}

esp_err_t webhookIF::start_connection(){
    m_tls = esp_tls_conn_http_new(m_webhook_uri, &m_cfg);

    if(m_tls == nullptr) {
        ESP_LOGE(TAG, "Connection failed");
        return ESP_ERR_ESP_TLS_FAILED_CONNECT_TO_HOST;
    }
    return ESP_OK;
}

esp_err_t webhookIF::write_request(const char *content, int content_length){
    // convert content_length to a char array
    char len[12];
    sprintf(len, "%d", content_length + 14); // 14 is the fixed length of the json wrapper

    //ESP_LOGI(TAG, "len %d", strlen(len));
    //ESP_LOGI(TAG, "wh %d", strlen(m_webhook_uri));

    int message_alloc_len = 154 + strlen(m_webhook_uri)+strlen(len)+content_length + 200;// remove the 200, just for easier development
    //ESP_LOGI(TAG, "Message alloc length %d", message_alloc_len);

    // construct message
    char *rqst = new char[message_alloc_len]; 
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
    //ESP_LOGI(TAG, "rqst len %d", strlen(rqst));

    size_t written_bytes = 0;
    int ret = 0;

    // Write https message to discord
    do {
        ret = esp_tls_conn_write(m_tls, 
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
        else{
            ESP_LOGE(TAG, "big oops right here");
        }
    } while(written_bytes < strlen(rqst));
    free(rqst);
    return ESP_OK;
}

esp_err_t webhookIF::print_response(){
    // Read http response
    esp_err_t retval = ESP_FAIL;
    int ret = 0;
    do
    {
        // buffer to read the http response into
        char buf[m_response_buf_size] = { 0 };

        int len = sizeof(buf) - 1;
        ret = esp_tls_conn_read(m_tls, (char *)buf, len);
        
        if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
            continue;
        
        if(ret < 0)
        {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
            retval = ESP_FAIL;
            break;
        }

        if(ret == 0)
        {
            //ESP_LOGI(TAG, "connection closed");
            retval = ESP_OK;
            break;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);
        /* Print response directly to stdout as it is read */
        for(int i = 0; i < len; i++) {
            putchar(buf[i]);
        }
    } while(1);
    return retval;
}

void webhookIF::close_connection(){
    esp_tls_conn_delete(m_tls);
}

esp_err_t webhookIF::send_message(const char *content, int content_length){
    // Start connection
    esp_err_t err = start_connection();
    if(err != ESP_OK){
        return err;
    }
    
    err = write_request(content, content_length);
    if(err != ESP_OK){
        return err;
    }

    close_connection();
    return ESP_OK;
}

esp_err_t webhookIF::send_message_print_response(const char *content, int content_length){
    // Start connection
    esp_err_t err = start_connection();
    if(err){
        return err;
    }
    
    err = write_request(content, content_length);
    if(err != ESP_OK){
        return err;
    }


    err = print_response();
    if(err){
        return err;
    }

    close_connection();
    return ESP_OK;

}
