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
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include "esp_netif.h"
#include <esp_vfs.h>
#include <esp_http_server.h>
#include <cJSON.h>

#include "manager.h"
#include "http_app.h"

/* @brief tag used for ESP serial console messages */
static const char TAG[] = "http_app";

/* @brief the HTTP server handle */
static httpd_handle_t httpd_handle = NULL;

/* function pointers to URI handlers that can be user made */
esp_err_t (*custom_get_httpd_uri_handler)(httpd_req_t *r) = NULL;
esp_err_t (*custom_post_httpd_uri_handler)(httpd_req_t *r) = NULL;

/* POST response context buffer*/
static char* context=NULL;

/* strings holding the URLs of the wifi manager */
static char* http_root_url = NULL;
static char* http_redirect_url = NULL;
static char* http_js_url = NULL;
static char* http_css_url = NULL;
static char* http_favicon_url = NULL;
static char* http_ap_url = NULL;
static char* http_connect_url = NULL;
static char* http_http_url = NULL;
static char* http_ipv4_url = NULL;
static char* http_wifi_url = NULL;
static char* http_client_ca_url = NULL;
static char* http_client_crt_url = NULL;
static char* http_client_key_url = NULL;
static char* http_wifi_ca_url = NULL;
static char* http_wifi_crt_url = NULL;
static char* http_wifi_key_url = NULL;

/**
 * @brief embedded binary data.
 * @see file "component.mk"
 * @see https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#embedding-binary-data
 */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[]   asm("_binary_style_css_end");
extern const uint8_t code_js_start[] asm("_binary_code_js_start");
extern const uint8_t code_js_end[] asm("_binary_code_js_end");


/* const httpd related values stored in ROM */
const static char http_200_hdr[] = "200 OK";
const static char http_302_hdr[] = "302 Found";
// const static char http_400_hdr[] = "400 Bad Request";
const static char http_404_hdr[] = "404 Not Found";
const static char http_503_hdr[] = "503 Service Unavailable";
const static char http_location_hdr[] = "Location";
const static char http_content_type_html[] = "text/html";
const static char http_content_type_js[] = "text/javascript";
const static char http_content_type_css[] = "text/css";
const static char http_content_type_json[] = "application/json";
const static char http_cache_control_hdr[] = "Cache-Control";
const static char http_cache_control_no_cache[] = "no-store, no-cache, must-revalidate, max-age=0";
const static char http_cache_control_cache[] = "public, max-age=31536000";
const static char http_pragma_hdr[] = "Pragma";
const static char http_pragma_no_cache[] = "no-cache";


static esp_err_t get_request_buffer(httpd_req_t *req, char* result){
	char buf[100];
	int ret, remaining = req->content_len;
	int end = 0;
	if (remaining >= SCRATCH_BUFSIZE) {
			/* Respond with 500 Internal Server Error */
			ESP_LOGE(TAG, "Content too long");
			httpd_resp_send_500(req);
			return ESP_FAIL;
	}
	while (remaining > 0) {
			/* Read the data for the request */
			if ((ret = httpd_req_recv(req, buf,
											MIN(remaining, sizeof(buf)))) <= 0) {
					if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
							/* Retry receiving if timeout occurred */
							continue;
					}
					return ESP_FAIL;
			}

			/* Send back the same data */
			httpd_resp_send_chunk(req, buf, ret);
			remaining -= ret;
			memcpy(result, buf, ret);
			result += ret;
			end = ret;
	}
	result[end] = '\0';
	return ESP_OK;
}

esp_err_t http_app_set_handler_hook( httpd_method_t method,  esp_err_t (*handler)(httpd_req_t *r)  ){

	if(method == HTTP_GET){
		custom_get_httpd_uri_handler = handler;
		return ESP_OK;
	}
	else if(method == HTTP_POST){
		custom_post_httpd_uri_handler = handler;
		return ESP_OK;
	}
	else{
		return ESP_ERR_INVALID_ARG;
	}
}

static esp_err_t http_server_post_handler(httpd_req_t *req){
	esp_err_t ret = ESP_OK;

	ESP_LOGI(TAG, "POST %s", req->uri);

	/* POST /client_ca.json */
	if(strcmp(req->uri, http_client_ca_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_http_ca(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /client_crt.json */
	if(strcmp(req->uri, http_client_crt_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_http_crt(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /client_key.json */
	if(strcmp(req->uri, http_client_key_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_http_key(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /wifi_ca.json */
	if(strcmp(req->uri, http_wifi_ca_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_wifi_ca(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /wifi_crt.json */
	if(strcmp(req->uri, http_wifi_crt_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_wifi_crt(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /wifi_key.json */
	if(strcmp(req->uri, http_wifi_key_url) == 0){
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_wifi_key(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /http_setup.json */
	else if(strcmp(req->uri, http_http_url) == 0) {
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_http_config(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /ipv4_setup.json */
	else if(strcmp(req->uri, http_ipv4_url) == 0) {
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_ipv4_config(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {			
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	/* POST /wifi_setup.json */
	else if(strcmp(req->uri, http_wifi_url) == 0) {
		ret = get_request_buffer(req, context);
		if(ret != ESP_OK) {
			httpd_resp_send_500(req);
		} else {
			ret = wifi_manager_save_wifi_config(context);
			if( ret != ESP_OK ){
					ESP_LOGE(TAG, "File write error: %s", esp_err_to_name(ret));
					httpd_resp_send_500(req);
			}else {		
				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_send(req, NULL, 0);
			}
		}
	}
	else{

		if(custom_post_httpd_uri_handler == NULL){
			httpd_resp_set_status(req, http_404_hdr);
			httpd_resp_send(req, NULL, 0);
		}
		else{

			/* if there's a hook, run it */
			ret = (*custom_post_httpd_uri_handler)(req);
		}
	}
	return ret;
}


static esp_err_t http_server_get_handler(httpd_req_t *req){

    char* host = NULL;
    size_t buf_len;
    esp_err_t ret = ESP_OK;

    ESP_LOGD(TAG, "GET %s", req->uri);

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
    	host = malloc(buf_len);
    	if(httpd_req_get_hdr_value_str(req, "Host", host, buf_len) != ESP_OK){
    		/* if something is wrong we just 0 the whole memory */
    		memset(host, 0x00, buf_len);
    	}
    }

	/* determine if Host is from the STA IP address */
	wifi_manager_lock_sta_ip_string(portMAX_DELAY);
	bool access_from_sta_ip = host != NULL?strstr(host, wifi_manager_get_sta_ip_string()):false;
	wifi_manager_unlock_sta_ip_string();


	if (host != NULL && !strstr(host, DEFAULT_AP_IP) && !access_from_sta_ip) {

		/* Captive Portal functionality */
		/* 302 Redirect to IP of the access point */
		httpd_resp_set_status(req, http_302_hdr);
		httpd_resp_set_hdr(req, http_location_hdr, http_redirect_url);
		httpd_resp_send(req, NULL, 0);

	}
	else{

		/* GET /  */
		if(strcmp(req->uri, http_root_url) == 0){
			httpd_resp_set_status(req, http_200_hdr);
			httpd_resp_set_type(req, http_content_type_html);
			httpd_resp_send(req, (char*)index_html_start, index_html_end - index_html_start);
		}
		/* GET /favicon.js */
		else if(strcmp(req->uri, http_favicon_url) == 0){
			httpd_resp_set_status(req, http_200_hdr);
			httpd_resp_set_type(req, http_content_type_js);
			httpd_resp_send(req, (char*)favicon_ico_start, favicon_ico_end - favicon_ico_start);
		}
		/* GET /code.js */
		else if(strcmp(req->uri, http_js_url) == 0){
			httpd_resp_set_status(req, http_200_hdr);
			httpd_resp_set_type(req, http_content_type_js);
			httpd_resp_send(req, (char*)code_js_start, code_js_end - code_js_start);
		}
		/* GET /style.css */
		else if(strcmp(req->uri, http_css_url) == 0){
			httpd_resp_set_status(req, http_200_hdr);
			httpd_resp_set_type(req, http_content_type_css);
			httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_cache);
			httpd_resp_send(req, (char*)style_css_start, style_css_end - style_css_start);
		}
		/* GET /ap.json */
		else if(strcmp(req->uri, http_ap_url) == 0){

			/* if we can get the mutex, write the last version of the AP list */
			if(wifi_manager_lock_json_buffer(( TickType_t ) 10)){

				httpd_resp_set_status(req, http_200_hdr);
				httpd_resp_set_type(req, http_content_type_json);
				httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
				httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
				char* ap_buf = wifi_manager_get_ap_list_json();
				httpd_resp_send(req, ap_buf, strlen(ap_buf));
				wifi_manager_unlock_json_buffer();
			}
			else{
				httpd_resp_set_status(req, http_503_hdr);
				httpd_resp_send(req, NULL, 0);
				ESP_LOGE(TAG, "http_server_netconn_serve: GET /ap.json failed to obtain mutex");
			}

			/* request a wifi scan */
			wifi_manager_scan_async();
		}
		/* GET /connect */
		else if(strcmp(req->uri, http_connect_url) == 0){
			httpd_resp_set_status(req, http_200_hdr);
			httpd_resp_send(req, NULL, 0);
			ESP_LOGI(TAG, "CONNECT...");
			wifi_manager_connect_async();
		}

	}

    /* memory clean up */
    if(host != NULL){
    	free(host);
    }

    return ret;

}

/* URI wild card for any GET request */
static const httpd_uri_t http_server_get_index_request = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_get_favicon_request = {
    .uri       = "/favicon.ico",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_get_style_request = {
    .uri       = "/style.css",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_get_code_request = {
    .uri       = "/code.js",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_get_ap_request = {
    .uri       = "/ap.json",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_get_connect_request = {
    .uri       = "/connect",
    .method    = HTTP_GET,
    .handler   = http_server_get_handler
};

static const httpd_uri_t http_server_post_client_ca_request = {
		.uri	= "/client_ca",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_client_crt_request = {
		.uri	= "/client_crt",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_client_key_request = {
		.uri	= "/client_key",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_wifi_ca_request = {
		.uri	= "/wifi_ca",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_wifi_crt_request = {
		.uri	= "/wifi_crt",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_wifi_key_request = {
		.uri	= "/wifi_key",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};

static const httpd_uri_t http_server_post_http_request = {
		.uri	= "/http_setup",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};					

static const httpd_uri_t http_server_post_ipv4_request = {
		.uri	= "/ipv4_setup",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};					

static const httpd_uri_t http_server_post_wifi_request = {
		.uri	= "/wifi_setup",
		.method = HTTP_POST,
		.handler = http_server_post_handler,
		.user_ctx = NULL
};					

void http_app_stop(){

	if(httpd_handle != NULL){

		/* dealoc context buffer*/
		if(context) {
			free(context);
			context = NULL;
		}

		/* dealloc URLs */
		if(http_root_url) {
			free(http_root_url);
			http_root_url = NULL;
		}
		if(http_redirect_url){
			free(http_redirect_url);
			http_redirect_url = NULL;
		}
		if(http_favicon_url){
			free(http_favicon_url);
			http_favicon_url = NULL;
		}
		if(http_js_url){
			free(http_js_url);
			http_js_url = NULL;
		}
		if(http_css_url){
			free(http_css_url);
			http_css_url = NULL;
		}
		if(http_ap_url){
			free(http_ap_url);
			http_ap_url = NULL;
		}
		if(http_connect_url){
			free(http_connect_url);
			http_connect_url = NULL;
		}
		if(http_http_url){
			free(http_http_url);
			http_http_url = NULL;
		}
		if(http_ipv4_url){
			free(http_ipv4_url);
			http_ipv4_url = NULL;
		}
		if(http_wifi_url){
			free(http_wifi_url);
			http_wifi_url = NULL;
		}
		if(http_client_ca_url){
			free(http_client_ca_url);
			http_client_ca_url = NULL;
		}
		if(http_client_crt_url){
			free(http_client_crt_url);
			http_client_crt_url = NULL;
		}
		if(http_client_key_url){
			free(http_client_key_url);
			http_client_key_url = NULL;
		}
		if(http_wifi_ca_url){
			free(http_wifi_ca_url);
			http_wifi_ca_url = NULL;
		}
		if(http_wifi_crt_url){
			free(http_wifi_crt_url);
			http_wifi_crt_url = NULL;
		}
		if(http_wifi_key_url){
			free(http_wifi_key_url);
			http_wifi_key_url = NULL;
		}

		/* stop server */
		httpd_stop(httpd_handle);
		httpd_handle = NULL;
	}
}


/**
 * @brief helper to generate URLs of the wifi manager
 */
static char* http_app_generate_url(const char* page){

	char* ret;

	int root_len = strlen(WEBAPP_LOCATION);
	const size_t url_sz = sizeof(char) * ( (root_len+1) + ( strlen(page) + 1) );

	ret = malloc(url_sz);
	memset(ret, 0x00, url_sz);
	strcpy(ret, WEBAPP_LOCATION);
	ret = strcat(ret, page);

	return ret;
}

void http_app_start(bool lru_purge_enable){

	esp_err_t err;

	if(httpd_handle == NULL){

		context = calloc(1, SCRATCH_BUFSIZE);
		if(!context) {
			ESP_LOGE(TAG, "No memory for context");
			return;
		}

		httpd_config_t config = HTTPD_DEFAULT_CONFIG();

		config.max_uri_handlers = 15;
		config.lru_purge_enable = lru_purge_enable;

		/* generate the URLs */
		if(http_root_url == NULL){
			int root_len = strlen(WEBAPP_LOCATION);

			/* all the pages */
			const char page_js[] = "code.js";
			const char page_css[] = "style.css";
			const char page_ico[] = "favicon.ico";
			const char page_ap[] = "ap.json";
			const char page_connect[] = "connect";
			const char page_http[] = "http_setup";
			const char page_ipv4[] = "ipv4_setup";
			const char page_wifi[] = "wifi_setup";
			const char page_client_ca[] = "client_ca";
			const char page_client_crt[] = "client_crt";
			const char page_client_key[] = "client_key";
			const char page_wifi_ca[] = "wifi_ca";
			const char page_wifi_crt[] = "wifi_crt";
			const char page_wifi_key[] = "wifi_key";

			/* root url, eg "/"   */
			const size_t http_root_url_sz = sizeof(char) * (root_len+1);
			http_root_url = malloc(http_root_url_sz);
			memset(http_root_url, 0x00, http_root_url_sz);
			strcpy(http_root_url, WEBAPP_LOCATION);

			/* redirect url */
			size_t redirect_sz = 22 + root_len + 1; /* strlen(http://255.255.255.255) + strlen("/") + 1 for \0 */
			http_redirect_url = malloc(sizeof(char) * redirect_sz);
			*http_redirect_url = '\0';

			if(root_len == 1){
				snprintf(http_redirect_url, redirect_sz, "http://%s", DEFAULT_AP_IP);
			}
			else{
				snprintf(http_redirect_url, redirect_sz, "http://%s%s", DEFAULT_AP_IP, WEBAPP_LOCATION);
			}

			ESP_LOGI(TAG, "ROOT URL: %s", http_root_url);

			/* generate the other pages URLs*/
			http_js_url = http_app_generate_url(page_js);
			http_favicon_url = http_app_generate_url(page_ico);
			http_css_url = http_app_generate_url(page_css);
			http_ap_url = http_app_generate_url(page_ap);
			http_connect_url = http_app_generate_url(page_connect);
			http_http_url = http_app_generate_url(page_http);
			http_ipv4_url = http_app_generate_url(page_ipv4);
			http_wifi_url = http_app_generate_url(page_wifi);
			http_client_ca_url = http_app_generate_url(page_client_ca);
			http_client_crt_url = http_app_generate_url(page_client_crt);
			http_client_key_url = http_app_generate_url(page_client_key);
			http_wifi_ca_url = http_app_generate_url(page_wifi_ca);
			http_wifi_crt_url = http_app_generate_url(page_wifi_crt);
			http_wifi_key_url = http_app_generate_url(page_wifi_key);
		}

		err = httpd_start(&httpd_handle, &config);

	    if (err == ESP_OK) {
	        ESP_LOGI(TAG, "Registering URI handlers");
	        httpd_register_uri_handler(httpd_handle, &http_server_get_index_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_get_favicon_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_get_style_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_get_code_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_get_ap_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_get_connect_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_client_ca_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_client_crt_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_client_key_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_wifi_ca_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_wifi_crt_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_wifi_key_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_http_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_ipv4_request);
	        httpd_register_uri_handler(httpd_handle, &http_server_post_wifi_request);
	    }
	}
}