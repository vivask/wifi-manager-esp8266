idf_component_register(SRCS "src/manager.c" 
                            "src/spiffs.c"
                            "src/flash.c"
                            "src/flashrw.c"
                            "src/storage.c"
                            "src/dns_server.c"
                            "src/http_app.c"
                            "src/http_server.c"
                            "src/http_handlers.c"
                            "src_http_client.c"
                            "src/ntp_client.c"
                            "src/ota.c"
                            "src/cb_list.c"
                        INCLUDE_DIRS include
                        EMBED_FILES ui/style.css ui/code.js ui/index.html ui/favicon.ico
                        # EMBED_FILES vue/style.css vue/code.js vue/index.html vue/favicon.ico
                        )
