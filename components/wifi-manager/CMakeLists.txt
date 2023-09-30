idf_component_register(SRCS "src/manager"
                    INCLUDE_DIRS include
                    COMPONENT_EMBED_TXTFILES certs/wpa2_ca.pem certs/wpa2_client.crt certs/wpa2_client.key)
