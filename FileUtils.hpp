//
// Created by garo on 15/01/2026.
//

#pragma once
#include <string>
#include <filesystem>

namespace FileUtils {
    std::string format_size(uintmax_t);
    std::string get_icon_name(const std::filesystem::path& path);
}