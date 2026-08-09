idf_component_register(SRCS "main.c" "MadgwickAHRS.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver lwip esp_event esp_wifi nvs_flash esp_driver_gpio)
