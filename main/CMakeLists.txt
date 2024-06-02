set(srcs "main.c")

idf_component_register(SRCS "sensor.c" "ble.c" "${srcs}"
                       INCLUDE_DIRS ".")
