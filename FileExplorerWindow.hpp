//
// Created by garo on 15/01/2026.
//

#pragma once
#include <gtkmm.h>
#include <filesystem>
#include <set>
#include "FileIconItem.hpp"

class FileExplorerWindow : public Gtk::Window {
public:
    FileExplorerWindow();

    enum class GroupBy {
        None,
        Type,
        Size,
        Date
    };

protected:

    void fill_menu_box(Gtk::Box& box);

    int on_universal_sort(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const;
    struct GroupInfo {
        std::string name;
        int sort_order;
    };
    GroupInfo get_group_info(const std::filesystem::path& path);
    GroupBy m_current_group_by = GroupBy::None;

    void create_context_menu(const std::filesystem::path& full_path);
    void on_flowbox_right_click(int n_press, double x, double y);

    //signals
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void on_home_clicked();
    void on_up_clicked();
    void on_menu_clicked();
    void on_tree_view_right_click(int n_press, double x, double y);
    void on_icon_item_activated(Gtk::FlowBoxChild* child);
    void on_view_switch_toggled();

    void refresh_file_list();
    void navigate_to(const std::filesystem::path& path);
    void open_or_navigate(const std::filesystem::path& path);

    enum class SortMode { Name, Size, type, Date };
    void sort_file_list(std::vector<std::filesystem::path>& entries) const;

    bool m_show_hidden = false;
    Glib::RefPtr<Gtk::EventControllerKey> m_KeyController;

    std::set<std::filesystem::path> m_pinned_paths;

    //  set_sort_column doesn't sort properly and ignores the . before the filename making all
    //  the hiden files be sorted with all the regular files
    int on_name_sort(const Gtk::TreeModel::const_iterator& a, const Gtk::TreeModel::const_iterator& b) const;

    void save_pins() const;

    void load_pins();

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> m_col_pinned;
        Gtk::TreeModelColumn<Glib::ustring> m_col_icon_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_size_text;
        Gtk::TreeModelColumn<unsigned long> m_col_size_real;
        Gtk::TreeModelColumn<Glib::ustring> m_col_date;
        Gtk::TreeModelColumn<Glib::ustring> m_col_type;
        Gtk::TreeModelColumn<bool> m_col_is_group_header;
        Gtk::TreeModelColumn<int> m_col_group_sort_id;

        ModelColumns() {
            add(m_col_pinned);
            add(m_col_icon_name);
            add(m_col_name);
            add(m_col_size_text);
            add(m_col_size_real);
            add(m_col_date);
            add(m_col_type);
            add(m_col_is_group_header);
            add(m_col_group_sort_id);
        }

    };

    ModelColumns m_Columns;

    Gtk::Box m_MainLayout;
    Gtk::Box m_Toolbar;

    Gtk::Stack m_Stack;
    Gtk::StackSwitcher m_StackSwitcher;

    Gtk::ScrolledWindow m_ScrolledWindowList;
    Gtk::TreeView m_TreeView;

    Gtk::ScrolledWindow m_ScrolledWindowIcons;
    Gtk::FlowBox m_FlowBox;

    Gtk::Button m_BtnHome;
    Gtk::Button m_BtnUp;
    Gtk::MenuButton m_BtnMenu;
    Gtk::ToggleButton m_BtnViewToggle;

    Glib::RefPtr<Gtk::TreeStore> m_refTreeModel;

    Gtk::Popover m_ContextMenu;
    Gtk::Box m_ContextMenuBox;

    Gtk::Popover m_RightClickPopover;
    Gtk::Box m_RightClickBox;

    Glib::RefPtr<Gtk::GestureClick> m_RightClickGesture;

    std::filesystem::path m_current_path;
};