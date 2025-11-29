#include "sd_driver.h"
#include "esp_log.h"

static const char* TAG = "SDCard";

/**
 * @brief Construct a new SDCard::SDCard object
 * 
 * @param miso  MISO Pin
 * @param mosi MOSI Pin
 * @param clk CLK Pin
 * @param cs CS Pin
 * @param host 
 */
SDCard::SDCard(int miso, int mosi, int clk, int cs, spi_host_device_t host)
    : pin_miso(miso), pin_mosi(mosi), pin_clk(clk), pin_cs(cs), host_slot(host)
{}

/**
 * @brief Destroy the SDCard::SDCard object
 * 
 */
SDCard::~SDCard() {
    unmount();
}

bool SDCard::mount(const char* mount_point) {
    if (mounted) return true;
    mountPoint = mount_point;

    ESP_LOGI(TAG, "Initializing SPI bus...");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .sclk_io_num = pin_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(host_slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize() failed: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "Configuring SDSPI device...");

    sdmmc_host_t host_cfg = SDSPI_HOST_DEFAULT();
    host_cfg.slot = host_slot;

    sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev_cfg.gpio_cs = (gpio_num_t)pin_cs;
    dev_cfg.host_id = host_slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdspi_mount(
        mountPoint.c_str(),
        &host_cfg,
        &dev_cfg,
        &mount_cfg,
        &card
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        spi_bus_free(host_slot);
        return false;
    }

    ESP_LOGI(TAG, "SD card mounted at %s", mountPoint.c_str());
    sdmmc_card_print_info(stdout, card);

    mounted = true;
    return true;
}

void SDCard::unmount() {
    if (!mounted) return;

    esp_vfs_fat_sdcard_unmount(mountPoint.c_str(), card);
    spi_bus_free(host_slot);

    mounted = false;
    ESP_LOGI(TAG, "SD card unmounted.");
}

bool SDCard::writeFile(const std::string& path, const std::string& data) {
    createDirectory(path);

    FILE* f = fopen(path.c_str(), "w");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for writing", path.c_str());
        return false;
    }

    fwrite(data.c_str(), 1, data.size(), f);

    fclose(f);
    return true;
}

bool SDCard::appendFile(const std::string& path, const std::string& data) {
    FILE* f = fopen(path.c_str(), "a");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for appending", path.c_str());
        return false;
    }
    
    fwrite(data.c_str(), 1, data.size(), f);

    fclose(f);
    return true;
}

std::string SDCard::readFile(const std::string& path) {
    std::string contents = "";

    FILE* f = fopen(path.c_str(), "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for reading", path.c_str());
        return contents;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), f)) {
        contents.append(buffer);
    }
    fclose(f);

    return contents;
}

std::vector<std::string> SDCard::listDirectory(const std::string& path) {
    std::vector<std::string> entries;

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open %s", path.c_str());
        return entries;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0) {
            entries.push_back(entry->d_name);
        }
    }

    closedir(dir);
    return entries;
}

bool SDCard::createDirectory(const std::string& path) {
    size_t pos = 0;

    while ((pos = path.find('/', pos)) != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty()) mkdir(dir.c_str(), 0777);
        pos++;
    }

    return true;
}