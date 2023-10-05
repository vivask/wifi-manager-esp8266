/**
 * author:  Viktar Vasiuk

   ----------------------------------------------------------------------
    Copyright (C) Viktar Vasiuk, 2023
    
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

@see https://github.com/vivask/wifi-manager
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <esp_http_server.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <cJSON.h>
#include <esp_heap_caps.h>

#include "manager.h"
#include "storage.h"
#include "http_handlers.h"

static const char *TAG = "rest_handlers";

static char* get_request_buffer(httpd_req_t *req, esp_err_t* err){
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        ESP_LOGE(TAG, "Content too long");
        httpd_resp_send_500(req);
        *err = ESP_FAIL;
        return NULL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            ESP_LOGE(TAG, "Failed to post control value");
            httpd_resp_send_500(req);
            *err = ESP_FAIL;
            return NULL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    *err = ESP_OK;
    return buf;
}

static void response(httpd_req_t *req, bool status, const char* message) {
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "message", message);
    cJSON_AddBoolToObject(root, "status", true);
    const char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, strlen(json_string));
    free((void *)json_string);
    cJSON_Delete(root);
}

