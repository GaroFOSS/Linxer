//
// Created by garo on 15/01/2026.
//

#include "FileUtils.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <unordered_map>

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

    std::string format_date(const std::filesystem::file_time_type &ftime) {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
        std::tm* gmt = std::localtime(&tt);

        std::stringstream ss;
        ss << std::put_time(gmt, "%Y-%m-%d %H:%M");
        return ss.str();
    }

    std::string_view get_file_type(const fs::path &path) {
        if (fs::is_directory(path)) return "Directory";
        std::string ext = path.extension().string();

        static const std::unordered_map<std::string, std::string> file_types = {
            {".txt", "Text File"},
            {".csv", "CSV"},
            {".json", "JSON"},
            {".cpp", "C++ Source"},
            {".hpp", "C++ Source"},
            {".cc",  "C++ Source"},
            {".png", "Image"},
            {".jpg", "Image"},
            {".jpeg", "Image"}
        };

        if (path.has_extension()) {
            auto it = file_types.find(ext);
            if (it != file_types.end()) {
                return it->second;
            }
        }
        if (!ext.empty()) {
            return ext.substr(1) + " File";
        }

        return "File";
    }

}
