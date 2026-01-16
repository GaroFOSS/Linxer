//
// Created by garo on 15/01/2026.
//

#pragma once
#include <gtkmm.h>
#include <filesystem>

class FileExplorerWindow : public Gtk::Window {
public:
    FileExplorerWindow();

protected:
    //signals
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void on_home_clicked();
    void on_up_clicked();
    void on_menu_clicked();

    void refresh_file_list();
    void navigate_to(const std::filesystem::path& path);

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_col_icon_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_size_text;
        Gtk::TreeModelColumn<unsigned long> m_col_size_real;

        ModelColumns() {
            add(m_col_icon_name);
            add(m_col_name);
            add(m_col_size_text);
            add(m_col_size_real);
        }

    };

    ModelColumns m_Columns;

    Gtk::Box m_MainLayout;
    Gtk::Box m_Toolbar;
    Gtk::ScrolledWindow m_ScrolledWindow;

    Gtk::Button m_BtnHome;
    Gtk::Button m_BtnUp;
    Gtk::Button m_BtnMenu;

    Gtk::TreeView m_TreeView;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

    std::filesystem::path m_current_path;
};