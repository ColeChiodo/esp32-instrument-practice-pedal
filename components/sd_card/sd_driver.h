#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <unistd.h> 

#include <dirent.h>
#include <sys/stat.h>

#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"

#include "BassHeroTabsBinary.h"

class SDCard {
    public:
        SDCard(int miso, int mosi, int clk, int cs, spi_host_device_t host = SPI2_HOST);
        ~SDCard();

        bool mount(const char* mount_point = "/sdcard");
        void unmount();

        bool writeFile(const std::string& path, const std::string& data);
        bool appendFile(const std::string& path, const std::string& data);
        std::string readFile(const std::string& path);
		Metadata getMetadata(const std::string& path);
		
		void scanZips(const std::string& path);
        std::vector<std::string> listDirectory(const std::string& path = "/sdcard");

    private:
        int pin_miso;
        int pin_mosi;
        int pin_clk;
        int pin_cs;
        spi_host_device_t host_slot;

        sdmmc_card_t* card = nullptr;
        std::string mountPoint;
        bool mounted = false;
        
		bool unzipFile(const std::string& path);
        bool createDirectory(const std::string& path);
};
