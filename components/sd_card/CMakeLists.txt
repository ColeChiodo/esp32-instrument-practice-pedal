idf_component_register(
    SRCS "sd_driver.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver sdmmc fatfs vfs
)
