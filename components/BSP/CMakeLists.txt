set(moduel_dirs IIC XL9555 SPI LCD CAN)

set(src_dirs ${moduel_dirs})

set(include_dirs ${moduel_dirs})

set(requires
            driver esp_timer esp_lcd lvgl esp_driver_twai)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)