//
// Created by garo on 02/02/2026.
//

#ifndef LINXER_FILEICONITEM_HPP
#define LINXER_FILEICONITEM_HPP
#include <filesystem>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <string>


class FileIconItem : public Gtk::Box {
public:
    FileIconItem(const std::string& name, const std::string& icon_name, const std::filesystem::path& path);
    std::filesystem::path get_path() const { return m_path;};

private:
    Gtk::Image m_icon;
    Gtk::Label m_label;
    std::filesystem::path m_path;

};


#endif //LINXER_FILEICONITEM_HPP