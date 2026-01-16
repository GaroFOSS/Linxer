//
// Created by garo on 15/01/2026.
//
#include "FileExplorerWindow.hpp"
#include "FileUtils.hpp"
#include <iostream>

namespace fs = std::filesystem;

FileExplorerWindow::FileExplorerWindow() {
    set_title("Linxer - The Linux Explorer");
    set_default_size(800, 500);

    //window layout
    m_ScrolledWindow.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    set_child(m_ScrolledWindow);
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

    pColumn->set_sort_column(m_Columns.m_col_icon_name);
    m_TreeView.append_column(*pColumn);

    int size_col_index = m_TreeView.append_column("Size", m_Columns.m_col_size_text);
    auto* pSizeColumn = m_TreeView.get_column(size_col_index -1);
    if (pSizeColumn) {
        pSizeColumn->set_sort_column(m_Columns.m_col_size_real);
    }

    //signals
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_row_activated));

    //initialize
    m_current_path = fs::current_path();
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
}

void FileExplorerWindow::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*){
    auto iter = m_refTreeModel->get_iter(path);
    if(iter){
        Glib::ustring name_u = (*iter)[m_Columns.m_col_name];
        std::string name = name_u.c_str();

        if(name == ".."){
            if (m_current_path.has_parent_path())
            {
                m_current_path = m_current_path.parent_path();
                refresh_file_list();
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
            std::cout << "WIP" << '\n';
        }


    }


}