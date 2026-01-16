//
// Created by garo on 15/01/2026.
//

#include "FileUtils.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

namespace FileUtils {
    std::string format_size(uintmax_t size) {
        if (size == 0) return "0 B";
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int i = 0;
        double dsize = static_cast<double>(size);
        while (dsize >= 1024 && i < 4) {
            dsize /= 1024;
            i++;
        }
        std::stringstream ss;
        ss <<std::fixed << std::setprecision(1)<< dsize << " " << units[i];
        return ss.str();
    };

    std::string get_icon_name(const fs::path& path) {
        if (fs::is_directory(path)) return "folder";
        return "text-x-generic";
    }
}