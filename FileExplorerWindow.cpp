//
// Created by garo on 15/01/2026.
//
#include "FileExplorerWindow.hpp"
#include "FileUtils.hpp"
#include <iostream>
#include <giomm.h>

namespace fs = std::filesystem;

FileExplorerWindow::FileExplorerWindow() : m_MainLayout(Gtk::Orientation::VERTICAL), m_Toolbar(Gtk::Orientation::HORIZONTAL) {
    set_title("Linxer - The Linux Explorer");
    set_default_size(800, 500);

    set_child(m_MainLayout);
    m_Toolbar.set_margin(5);
    m_Toolbar.set_spacing(5);

    m_BtnHome.set_icon_name("go-home");
    m_BtnHome.set_tooltip_text("Home directory");

    m_BtnUp.set_icon_name("go-up");
    m_BtnUp.set_tooltip_text("Up Directory");

    m_BtnMenu.set_icon_name("open-menu");
    m_BtnMenu.set_tooltip_text("Menu");

    m_Toolbar.append(m_BtnHome);
    m_Toolbar.append(m_BtnUp);
    m_Toolbar.append(m_BtnMenu);

    m_MainLayout.append(m_Toolbar);

    //window layout
    m_ScrolledWindow.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    m_ScrolledWindow.set_expand(true);
    m_MainLayout.append(m_ScrolledWindow);

    m_ScrolledWindow.set_child(m_TreeView);

    //window model
    m_refTreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);

    auto* pColumn = Gtk::make_managed<Gtk::TreeViewColumn>("Filename");

    //cell icon
    auto* pCellIcon = Gtk::make_managed<Gtk::CellRendererPixbuf>();
    pColumn->pack_start(*pCellIcon, false); //Test on Expand
    pColumn->add_attribute(pCellIcon->property_icon_name(), m_Columns.m_col_icon_name);

    //cell text
    auto* pCellText = Gtk::make_managed<Gtk::CellRendererText>();
    pColumn->pack_start(*pCellText, true);
    pColumn->add_attribute(pCellText->property_text(), m_Columns.m_col_name);

    pColumn->set_sort_column(m_Columns.m_col_name);
    m_TreeView.append_column(*pColumn);

    int size_col_index = m_TreeView.append_column("Size", m_Columns.m_col_size_text);
    auto* pSizeColumn = m_TreeView.get_column(size_col_index -1);
    if (pSizeColumn) {
        pSizeColumn->set_sort_column(m_Columns.m_col_size_real);
    }

    //signals
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_row_activated));
    m_BtnHome.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_home_clicked));
    m_BtnUp.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_up_clicked));
    m_BtnMenu.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_menu_clicked));

    //initialize
    const char* home_dir = std::getenv("HOME");
    m_current_path = home_dir ? fs::path(home_dir) : fs::current_path();
    refresh_file_list();
}

void FileExplorerWindow::refresh_file_list(){
    m_refTreeModel->clear();

    if(m_current_path != m_current_path.root_path()){
        auto row = *(m_refTreeModel->append());
        row[m_Columns.m_col_icon_name] = "go-up";
        row[m_Columns.m_col_name] = "..";
        row[m_Columns.m_col_size_text] = "";
        row[m_Columns.m_col_size_real] = 0;
    }
    try {
        for(const auto& entry : fs::directory_iterator(m_current_path)) {
            auto row = *(m_refTreeModel->append());
            row[m_Columns.m_col_icon_name] = FileUtils::get_icon_name(entry.path().string());
            row[m_Columns.m_col_name] = entry.path().filename().string();

            if (entry.is_regular_file()) {
                uintmax_t size = entry.file_size();
                row[m_Columns.m_col_size_text] = FileUtils::format_size(size);
                row[m_Columns.m_col_size_real] = size;
            } else {
                row[m_Columns.m_col_size_text] = "FileUtils::format_size(size)";
                row[m_Columns.m_col_size_real] = 0;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
    }
    set_title(m_current_path.c_str());

    m_BtnUp.set_sensitive(m_current_path != m_current_path.root_path());
}

void FileExplorerWindow::navigate_to(const fs::path& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        try {
            fs::directory_iterator check_access(path);
            m_current_path = path;
            refresh_file_list();
        } catch (const fs::filesystem_error& e) {
            std::cerr << e.what() << '\n';
        }
    }
}

void FileExplorerWindow::on_home_clicked() {
    const char* home_dir = std::getenv("HOME");
    if (home_dir) navigate_to(home_dir);
}

void FileExplorerWindow::on_up_clicked() {
    if (m_current_path.has_parent_path()) navigate_to(m_current_path.parent_path());
}

void FileExplorerWindow::on_menu_clicked() {
    puts("WIP");
}

void FileExplorerWindow::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*){
    auto iter = m_refTreeModel->get_iter(path);
    if(iter){
        Glib::ustring name_u = (*iter)[m_Columns.m_col_name];
        std::string name = name_u.c_str();

        if(name == ".."){
            if (m_current_path.has_parent_path())
            {
                navigate_to(m_current_path.parent_path());
            }
            return;
        }
        fs::path new_path = m_current_path / name;

        if(fs::is_directory(new_path)){
            try
            {
                fs::directory_iterator check_access(new_path);
                m_current_path = new_path;
                refresh_file_list();
            }
            catch(const fs::filesystem_error& e)
            {
                std::cerr << e.what() << '\n' << name << " is invalid";
            }
        }
        else
        {
            try {
                auto file_obj = Gio::File::create_for_path(new_path.string());
                std::string uri = file_obj->get_uri();
                Gio::AppInfo::launch_default_for_uri(uri);
            } catch (const Glib::Error& e) {
                std::cerr << "Glib file opening error: " << e.what() << '\n';
            } catch (const std::exception& e) {
                std::cerr << "standard file opening error" << e.what() << '\n';
            }
        }
    }
}