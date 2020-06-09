extern "C" {
    void app_main(void);
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "webhookIF.hpp"

extern const uint8_t *root_cert_start asm("_binary_server_root_cert_pem_start");
extern const uint8_t *root_cert_end   asm("_binary_server_root_cert_pem_end");

static const char *webhook_url = "https://discord.com/api/webhooks/719008915194642443/Ubp9z0s0CxoC2z5gd6FboaOPB1iDpqPE9mJU51oyrQUqDFRmtHgSl-WXwGc-xsp4Ncbq";
void app_main(void)
{
    webhookIF discord(webhook_url, root_cert_start, root_cert_end);
    discord.sendMessage("hey dawg", 8);
}
