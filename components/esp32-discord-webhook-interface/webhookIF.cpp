#include "webhookIF.hpp"

const char * webhookIF::TAG = "webhookIF";

webhookIF::webhookIF(const char *webhookURL, 
    const uint8_t *binary_server_root_cert_begin, const uint8_t *binary_server_root_cert_end){
        
}

esp_err_t webhookIF::sendMessage(const char *content, int content_length){
    if(strlen(content) == content_length){
        ESP_LOGI(TAG, "Length matches. Message: %s", content);
        return ESP_OK;
    }
    else{
        ESP_LOGE(TAG, "Length don't matches. Message: %s", content);
        return ESP_ERR_INVALID_SIZE;
    }

}