#pragma once   // Prevents double inclusion

#include <string>
#include <vector>
#include <cstdint>

// --- META file ---
struct Metadata {
    char magic[4];
	uint8_t version;
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string tab_author;
};
