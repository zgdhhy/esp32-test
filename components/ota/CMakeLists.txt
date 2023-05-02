idf_component_register(SRCS "myOTA.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_https_ota app_update)