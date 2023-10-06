/**
 * original author:  Tony Pottier
 * modification: 	 Viktar Vasiuk

   ----------------------------------------------------------------------
    Copyright (C) Viktar Vasiuk, 2023
   	Copyright (C) Tony Pottier, 2007
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------

@see https://github.com/tonyp7/esp32-wifi-manager
@see https://github.com/vivask/esp8266-wifi-manager
 */
#pragma once

#include <stdbool.h>
#include <esp_err.h>
#include <esp_http_server.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define SCRATCH_BUFSIZE (10240)
#define SCRATCH_BUFSIZE (4096)

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
