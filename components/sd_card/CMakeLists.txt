idf_component_register(
    SRCS "sd_driver.cpp"
    INCLUDE_DIRS "." "../miniz"
    REQUIRES driver sdmmc fatfs vfs miniz
)
