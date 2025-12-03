#include "sd_driver.h"
#include "esp_log.h"
extern "C" {
#include "miniz.h"
}

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
	    .mosi_io_num = GPIO_NUM_23,
	    .miso_io_num = GPIO_NUM_19,
	    .sclk_io_num = GPIO_NUM_18,
	    .quadwp_io_num = -1,
	    .quadhd_io_num = -1,
	    .data4_io_num = -1,
	    .data5_io_num = -1,
	    .data6_io_num = -1,
	    .data7_io_num = -1,
	    .data_io_default_level = 0,
	    .max_transfer_sz = 4000,
	    .flags = 0,
	    .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
	    .intr_flags = 0
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

	esp_vfs_fat_mount_config_t mount_cfg = {
	    .format_if_mount_failed = false,
	    .max_files = 5,
	    .allocation_unit_size = 16 * 1024,
	    .disk_status_check_enable = false,
	    .use_one_fat = false
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

void SDCard::getMetadata(const std::string& path) {
	
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

void SDCard::scanZips(const std::string& path) {
	ESP_LOGI(TAG, "Scanning %s for zip files", path.c_str());
	const std::vector<std::string> files = listDirectory(path);
    for (const std::string& f : files) {
		if (f.ends_with(".zip")) {
			unzipFile(path + "/" + f);
		}
    } 
}

bool SDCard::unzipFile(const std::string& path) {
	ESP_LOGI(TAG, "Zip file found: %s | Unzipping", path.c_str());
    const char* zip_path = path.c_str();
    std::string output_dir = path;
    size_t pos = output_dir.rfind('.');
    if (pos != std::string::npos) {
        output_dir = output_dir.substr(0, pos);
    }

    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));
	
    if (!mz_zip_reader_init_file(&zip_archive, zip_path, 0)) {
        fprintf(stderr, "Failed to open ZIP: %s\n", zip_path);
        return false;
    }

    mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);

    for (mz_uint i = 0; i < num_files; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            fprintf(stderr, "Failed to get info for file index %lu\n", (unsigned long)i);
            continue;
        }

        std::string full_path = output_dir + "/" + file_stat.m_filename;

        if (file_stat.m_is_directory) {
            createDirectory(full_path); // your helper
            continue;
        }

        if (!mz_zip_reader_extract_to_file(&zip_archive, i, full_path.c_str(), 0)) {
            fprintf(stderr, "Failed to extract file: %s\n", full_path.c_str());
        }
    }

    mz_zip_reader_end(&zip_archive);

    if (unlink(zip_path) != 0) {
        perror("Failed to delete ZIP");
        return false;
    }

    return true;
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

