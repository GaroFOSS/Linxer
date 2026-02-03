//
// Created by garo on 02/02/2026.
//

#include "FileIconItem.hpp"

FileIconItem::FileIconItem(const std::string &name, const std::string &icon_name, const std::filesystem::path &path) : Gtk::Box(Gtk::Orientation::VERTICAL, 5), m_path(path){

    m_icon.set_from_icon_name(icon_name);
    m_icon.set_pixel_size(64);
    m_icon.set_tooltip_text(m_path.string());
    m_icon.set_halign(Gtk::Align::CENTER);
    append(m_icon);

    m_label.set_text(name);
    m_label.set_ellipsize(Pango::EllipsizeMode::END);
    m_label.set_max_width_chars(12);
    m_label.set_wrap(true);
    m_label.set_halign(Gtk::Align::CENTER);
    m_label.set_valign(Gtk::Align::START);
    append(m_label);

    set_margin(10);

}