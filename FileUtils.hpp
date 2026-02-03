//
// Created by garo on 15/01/2026.
//

#pragma once
#include <string>
#include <filesystem>

namespace FileUtils {
    std::string format_size(uintmax_t);
    std::string get_icon_name(const std::filesystem::path& path);
    std::string format_date(const std::filesystem::file_time_type& ftime);
    std::string_view get_file_type(const std::filesystem::path& path);
}