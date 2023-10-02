#pragma once

#include <stdbool.h>
#include <esp_http_server.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @brief Defines the URL where the wifi manager is located
 *  By default it is at the server root (ie "/"). If you wish to add your own webpages
 *  you may want to relocate the wifi manager to another URL, for instance /wifimanager
 */
#define WEBAPP_LOCATION 					CONFIG_WEBAPP_LOCATION


/** 
 * @brief spawns the http server 
 */
void http_app_start(bool lru_purge_enable);

/**
 * @brief stops the http server 
 */
void http_app_stop();

/** 
 * @brief sets a hook into the wifi manager URI handlers. Setting the handler to NULL disables the hook.
 * @return ESP_OK in case of success, ESP_ERR_INVALID_ARG if the method is unsupported.
 */
esp_err_t http_app_set_handler_hook( httpd_method_t method,  esp_err_t (*handler)(httpd_req_t *r)  );


#ifdef __cplusplus
}
#endif
