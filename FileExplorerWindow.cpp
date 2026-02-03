//
// Created by garo on 15/01/2026.
//
#include "FileExplorerWindow.hpp"
#include "FileIconItem.hpp"

#include <fstream>

#include "FileUtils.hpp"
#include <iostream>
#include <giomm.h>

namespace fs = std::filesystem;

FileExplorerWindow::FileExplorerWindow()
        : m_MainLayout(Gtk::Orientation::VERTICAL),
        m_Toolbar(Gtk::Orientation::HORIZONTAL),
        m_ContextMenuBox(Gtk::Orientation::VERTICAL),
        m_RightClickBox(Gtk::Orientation::VERTICAL){
    set_title("Linxer - The Linux Explorer");
    set_default_size(800, 500);

    set_child(m_MainLayout);
    m_Toolbar.set_margin(5);
    m_Toolbar.set_spacing(5);

    m_BtnHome.set_icon_name("go-home");
    m_BtnHome.set_tooltip_text("Home directory");

    m_BtnUp.set_icon_name("go-up");
    m_BtnUp.set_tooltip_text("Up Directory");

    m_BtnViewToggle.set_icon_name("view-grid-symbolic");
    m_BtnViewToggle.set_tooltip_text("Switch to Icon View");

    m_BtnMenu.set_icon_name("view-more-symbolic");
    m_BtnMenu.set_tooltip_text("Menu");

    m_Toolbar.append(m_BtnHome);
    m_Toolbar.append(m_BtnUp);

    auto* spacer = Gtk::make_managed<Gtk::Box>();
    spacer->set_hexpand(true);
    m_Toolbar.append(*spacer);

    m_Toolbar.append(m_BtnViewToggle);
    m_Toolbar.append(m_BtnMenu);

    m_MainLayout.append(m_Toolbar);

    m_MainLayout.append(m_Stack);
    m_Stack.set_transition_type(Gtk::StackTransitionType::CROSSFADE);
    m_Stack.set_expand(true);

    load_pins();

    //window layout
    m_ScrolledWindowList.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    m_ScrolledWindowList.set_child(m_TreeView);
    m_Stack.add(m_ScrolledWindowList, "list_view", "List");

    m_ScrolledWindowIcons.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    m_ScrolledWindowIcons.set_child(m_FlowBox);
    m_Stack.add(m_ScrolledWindowIcons, "icon_view", "Icons");

    m_FlowBox.set_valign(Gtk::Align::START);
    m_FlowBox.set_max_children_per_line(10);
    m_FlowBox.set_selection_mode(Gtk::SelectionMode::SINGLE);
    m_FlowBox.set_activate_on_single_click(false);


    //window model
    m_refTreeModel = Gtk::TreeStore::create(m_Columns);
    m_TreeView.set_model(m_refTreeModel);

    auto sortable = std::dynamic_pointer_cast<Gtk::TreeSortable>(m_refTreeModel);
    if (sortable) {
        sortable->set_default_sort_func(
            sigc::mem_fun(*this, &FileExplorerWindow::on_universal_sort));
        sortable->set_sort_column(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SortType::ASCENDING);
    }

    auto* pPinColumn = Gtk::make_managed<Gtk::TreeViewColumn>("Pinned");
    auto* pCellPin = Gtk::make_managed<Gtk::CellRendererPixbuf>();
    pPinColumn->pack_start(*pCellPin, false);
    pPinColumn->add_attribute(pCellPin->property_icon_name(), m_Columns.m_col_pinned);
    m_TreeView.append_column(*pPinColumn);


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
    m_TreeView.append_column(*pColumn);/**/

    int size_col_index = m_TreeView.append_column("Size", m_Columns.m_col_size_text);
    auto* pSizeColumn = m_TreeView.get_column(size_col_index -1);
    if (pSizeColumn) {
        pSizeColumn->set_sort_column(m_Columns.m_col_size_real);
    }

    int type_idx = m_TreeView.append_column("Type", m_Columns.m_col_type);
    m_TreeView.get_column(type_idx -1)->set_sort_column(m_Columns.m_col_type);

    int date_idx = m_TreeView.append_column("Date Modified", m_Columns.m_col_date);
    m_TreeView.get_column(date_idx -1)->set_sort_column(m_Columns.m_col_date);

    //signals
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_row_activated));
    m_FlowBox.signal_child_activated().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_icon_item_activated));
    m_BtnHome.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_home_clicked));
    m_BtnUp.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_up_clicked));
    // m_BtnMenu.signal_clicked().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_menu_clicked));
    m_BtnViewToggle.signal_toggled().connect(sigc::mem_fun(*this, &FileExplorerWindow::on_view_switch_toggled));


    m_BtnMenu.set_popover(m_ContextMenu);
    m_ContextMenu.set_child(m_ContextMenuBox);
    fill_menu_box(m_ContextMenuBox);

    m_RightClickPopover.set_child(m_RightClickBox);
    m_RightClickPopover.set_has_arrow(false);
    m_RightClickPopover.set_halign(Gtk::Align::START);
    // create_contex_menu();

    m_TreeView.set_enable_search(false);
    m_FlowBox.set_can_focus(true);

    // Key Controller
    m_KeyController = Gtk::EventControllerKey::create();
    m_KeyController->signal_key_pressed().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if ((keyval == GDK_KEY_h || keyval == GDK_KEY_H) && ((state & Gdk::ModifierType::CONTROL_MASK) == Gdk::ModifierType::CONTROL_MASK)) {
                m_show_hidden = !m_show_hidden;
                refresh_file_list();
                return true;
            }
            return false;
        }, false);

    add_controller(m_KeyController);

    //TreeView
    auto tree_gesture = Gtk::GestureClick::create();
    tree_gesture->set_button(3);
    tree_gesture->signal_pressed().connect([this](int n_pressed, double x, double y) {
        on_tree_view_right_click(n_pressed, x, y);
    });
    m_TreeView.add_controller(tree_gesture);

    //FlowBox
    auto icon_gesture = Gtk::GestureClick::create();
    icon_gesture->set_button(3);
    icon_gesture->signal_pressed().connect(
        [this](int n_pressed, double x, double y) {
            on_flowbox_right_click(n_pressed, x, y);
        });
    m_FlowBox.add_controller(icon_gesture);


    //initialize
    const char* home_dir = std::getenv("HOME");
    m_current_path = home_dir ? fs::path(home_dir) : fs::current_path();
    refresh_file_list();
}

void FileExplorerWindow::on_view_switch_toggled() {
    if (m_BtnViewToggle.get_active()) {
        m_Stack.set_visible_child("icon_view");
        m_BtnViewToggle.set_icon_name("view-list-symbolic");
        m_BtnViewToggle.set_tooltip_markup("Switch to Grid View");
    }
    else {
        m_Stack.set_visible_child("list_view");
        m_BtnViewToggle.set_icon_name("view-grid-symbolic");
        m_BtnViewToggle.set_tooltip_markup("Switch to Icon View");
    }
}

void FileExplorerWindow::fill_menu_box(Gtk::Box& box) {
    while (auto* child = box.get_first_child()) {
        box.remove(*child);
    }

    box.set_margin(10);

    auto* hidden_toggle = Gtk::make_managed<Gtk::CheckButton>("Show Hidden Files");
    hidden_toggle->set_active(m_show_hidden);
    hidden_toggle->signal_toggled().connect([this, hidden_toggle]() {
        m_show_hidden = hidden_toggle->get_active();
        refresh_file_list(); // Reload the view immediately
    });
    box.append(*hidden_toggle);

    auto* separator_hidden_group_by = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    separator_hidden_group_by->set_margin_top(10);
    separator_hidden_group_by->set_margin_bottom(10);
    box.append(*separator_hidden_group_by);

    auto* label_group = Gtk::make_managed<Gtk::Label>("<b>Group By</b>");
    label_group->set_use_markup(true);
    label_group->set_halign(Gtk::Align::START);
    box.append(*label_group);

    auto* rb_none = Gtk::make_managed<Gtk::CheckButton>("None");
    rb_none->set_active(m_current_group_by == GroupBy::None);
    rb_none->signal_toggled().connect([this, rb_none]() {
        if (rb_none->get_active()) {
            m_current_group_by = GroupBy::None;
            refresh_file_list();
        }
    });
    box.append(*rb_none);

    auto* rb_type = Gtk::make_managed<Gtk::CheckButton>("Type");
    rb_type->set_group(*rb_none);
    rb_type->set_active(m_current_group_by == GroupBy::Type);
    rb_type->signal_toggled().connect([this, rb_type]() {
        if (rb_type->get_active()) {
            m_current_group_by = GroupBy::Type;
            refresh_file_list();
        }
    });
    box.append(*rb_type);

    auto* rb_size = Gtk::make_managed<Gtk::CheckButton>("Size");
    rb_size->set_group(*rb_none);
    rb_size->set_active(m_current_group_by == GroupBy::Size);
    rb_size->signal_toggled().connect([this, rb_size]() {
        if (rb_size->get_active()) {
            m_current_group_by = GroupBy::Size;
            refresh_file_list();
        }
    });
    box.append(*rb_size);

    auto* separator_group_by_columns = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
    separator_group_by_columns->set_margin_top(10);
    separator_group_by_columns->set_margin_bottom(10);
    box.append(*separator_group_by_columns);

    auto* label = Gtk::make_managed<Gtk::Label>("<b>Columns</b>");
    label->set_use_markup(true);
    label->set_halign(Gtk::Align::START);
    box.append(*label);

    unsigned int n_columns = m_TreeView.get_n_columns();
    for (int i = 0; i < n_columns; i++) {
        auto* col = m_TreeView.get_column(i);
        Glib::ustring title = col->get_title();

        if (title.empty()) continue;

        auto* check = Gtk::make_managed<Gtk::CheckButton>(title);
        check->set_active(col->get_visible());

        check->signal_toggled().connect(
            [col,check]() {
                col->set_visible(check->get_active());
            });
        box.append(*check);
    }
}

int FileExplorerWindow::on_universal_sort(const Gtk::TreeModel::const_iterator &a,
    const Gtk::TreeModel::const_iterator &b) const {
    if (!a || !b) return 0;

    int depth = m_refTreeModel->iter_depth(a);
    if (depth == 0 && m_current_group_by != GroupBy::None) {
        int id_a = (*a)[m_Columns.m_col_group_sort_id];
        int id_b = (*b)[m_Columns.m_col_group_sort_id];

        if (id_a != id_b) return id_a - id_b;

        Glib::ustring name_a = (*a)[m_Columns.m_col_name];
        Glib::ustring name_b = (*b)[m_Columns.m_col_name];
        return name_a.compare(name_b);
    }
    else {
        int sort_col_id;
        Gtk::SortType sort_order;

        auto sortable = std::dynamic_pointer_cast<Gtk::TreeSortable>(m_refTreeModel);
        if (sortable->get_sort_column_id(sort_col_id, sort_order)) {
            if (sort_col_id == m_Columns.m_col_size_real.index()) {
                auto size_a = (*a)[m_Columns.m_col_size_real];
                auto size_b = (*b)[m_Columns.m_col_size_real];
                int result = (size_a < size_b) ? -1 : (size_a > size_b) ? 1 : 0;
                return (sort_order == Gtk::SortType::ASCENDING) ? result : -result;
            }
        }

        return on_name_sort(a,b);
    }
}

FileExplorerWindow::GroupInfo FileExplorerWindow::get_group_info(const std::filesystem::path &path) {
    GroupInfo info;
    std::error_code ec;

    if (m_current_group_by == GroupBy::Size) {
        if (fs::is_directory(path, ec)) {
            info.name = "Folders";
            info.sort_order = 0;
        }else {
            const auto size = fs::file_size(path, ec);
            if (size < 1024) {info.name = "Tiny (0 - 1 KB)"; info.sort_order = 1;}
            else if (size < pow(1024, 2)) { info.name = "Small (1 KB - 1 MB"; info.sort_order = 2;}
            else if (size < 100 * pow(1024,2)) { info.name = "Medium (1 MB - 100 MB"; info.sort_order = 3;}
            else if (size < pow(1024, 3)) { info.name = "Large (100 MB - 1 GB"; info.sort_order = 4;}
            else { info.name = "Huge (> 1 GB)"; info.sort_order = 5;}

        }
    }
    else if (m_current_group_by == GroupBy::Type) {
        if (fs::is_directory(path, ec)) {
            info.name = "Folders";
            info.sort_order = 0;
        }else {
            std::string ext = path.extension().string();
            if (ext.empty()) { info.name = "File"; info.sort_order = 99; }
            else { info.name = ext + " File"; info.sort_order = 1; }
        }
    }
    else {
        info.name = "Files";
        info.sort_order = 0;
    }
    return info;
}

void FileExplorerWindow::create_context_menu(const fs::path &full_path) {
    while (auto* child = m_RightClickBox.get_first_child()) {
        m_RightClickBox.remove(*child);
    }

    bool is_pinned = m_pinned_paths.count(full_path);
    auto* pin_btn = Gtk::make_managed<Gtk::Button>(is_pinned ? "Unpin from top" : "Pin on Top");
    pin_btn->set_has_frame(false);
    pin_btn->set_halign(Gtk::Align::START);
    pin_btn->signal_clicked().connect([this, full_path, is_pinned]() {
        if (is_pinned) m_pinned_paths.erase(full_path);
        else m_pinned_paths.insert(full_path);
        save_pins();
        refresh_file_list();
        m_RightClickPopover.popdown();
    });

    m_RightClickBox.append(*pin_btn);

    auto* del_btn = Gtk::make_managed<Gtk::Button>("Move to Trash");
    del_btn->set_has_frame(false);
    del_btn->set_halign(Gtk::Align::START);

    del_btn->signal_clicked().connect([this, full_path]() {
        try {
            auto file = Gio::File::create_for_path(full_path.string());
            if (file->trash()) {
                puts(("Moved to Trash: " + full_path.string() + "\n").c_str() );
                refresh_file_list();
            }
        }
        catch (const Glib::Error& ex) {
            std::cerr << ex.what() << std::endl;
        }

        m_RightClickPopover.popdown();
    });

    m_RightClickBox.append(*del_btn);
}

void FileExplorerWindow::on_flowbox_right_click(int n_press, double x, double y) {
    auto* child = m_FlowBox.get_child_at_pos((int)x, (int)y);
    if (child) {
        m_FlowBox.select_child(*child);
        auto* item = dynamic_cast<FileIconItem*>(child->get_child());
        if (item) {
            fs::path full_path = item->get_path();
            create_context_menu(full_path);

            const Gdk::Rectangle rect((int)x, (int)y, 1, 1);
            m_RightClickPopover.set_parent(m_FlowBox);
            m_RightClickPopover.set_pointing_to(rect);
            m_RightClickPopover.popup();
        }
    else {
        m_FlowBox.unselect_all();
    }
    }
}

void FileExplorerWindow::on_tree_view_right_click(int n_press, double x, double y) {
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* column;
    int cell_x, cell_y, bin_x, bin_y;

    m_TreeView.convert_widget_to_bin_window_coords((int)x, (int)y, bin_x, bin_y); //AAAAAAAAAAAAAAAAAAAAAAAAAAAA GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF GO FUCK YOURSELF

    if (m_TreeView.get_path_at_pos(bin_x, bin_y, path, column, cell_x, cell_y)) {
        auto iter = m_refTreeModel->get_iter(path);
        if (iter) {
            Glib::ustring name = (*iter)[m_Columns.m_col_name];
            fs::path full_path = m_current_path/name.c_str();

            create_context_menu(full_path);

            const Gdk::Rectangle rect(static_cast<int>(x), static_cast<int>(y), 1,1);
            m_RightClickPopover.set_parent(m_TreeView);
            m_RightClickPopover.set_pointing_to(rect);
            m_RightClickPopover.popup();
        }
    }
}

void FileExplorerWindow::refresh_file_list(){
    m_refTreeModel->clear();

    std::map<std::string, Gtk::TreeModel::Row> group_rows;

    while (auto* child = m_FlowBox.get_first_child()) {
        m_FlowBox.remove(*child);
    }

    try {
        std::vector<fs::path> entries;
        for (const auto& entry : fs::directory_iterator(m_current_path)) {
            std::string filename = entry.path().filename().string();
            if (!m_show_hidden && !filename.empty() && filename[0] == '.') {
                continue;
            }
            entries.push_back(entry.path());
        }

        std::sort(entries.begin(), entries.end(), [this](const fs::path& a, const fs::path& b) {
            bool pinned_a = m_pinned_paths.count(a);
            bool pinned_b = m_pinned_paths.count(b);
            if (pinned_a != pinned_b) return pinned_a;

            std::error_code ec;
            bool is_dir_a = fs::is_directory(a, ec);
            bool is_dir_b = fs::is_directory(b, ec);

            if (is_dir_a != is_dir_b) return is_dir_a;

            std::string name_a = a.filename().string();
            std::string name_b = b.filename().string();

            bool hidden_a = !name_a.empty() && name_a[0] == '.';
            bool hidden_b = !name_b.empty() && name_b[0] == '.';
            if (hidden_a != hidden_b) return hidden_a;

            return name_a < name_b;
        });

        for(const auto& path : entries) {

            Gtk::TreeModel::Row parent_row;

            if (m_current_group_by != GroupBy::None) {
                GroupInfo g_info = get_group_info(path);
                auto it_group = group_rows.find(g_info.name);

                if (it_group == group_rows.end()) {
                    auto row = *(m_refTreeModel->append());
                    row[m_Columns.m_col_name] = g_info.name;
                    row[m_Columns.m_col_is_group_header] = true;
                    row[m_Columns.m_col_group_sort_id] = g_info.sort_order;
                    row[m_Columns.m_col_icon_name] = "folder-open-symbolic";
                    group_rows[g_info.name] = row;
                    parent_row = row;
                }
                else {
                    parent_row = it_group->second;
                }
            }

            Gtk::TreeModel::Row row;
            if (m_current_group_by != GroupBy::None) {
                row = *(m_refTreeModel->append(parent_row.children()));
            } else {
                row = *(m_refTreeModel->append());
            }

            std::string name = path.filename().string();
            std::string icon = FileUtils::get_icon_name(path);
            std::string icon_display_name = name;

            if (m_pinned_paths.count(path)) {
                icon_display_name = "📌 " + name;
            }

            if (m_pinned_paths.count(path)) {
                row[m_Columns.m_col_pinned] = "view-pin-symbolic";
            }
            else {
                row[m_Columns.m_col_pinned] = "";
            }

            row[m_Columns.m_col_icon_name] = icon;
            row[m_Columns.m_col_name] = name;

            std::error_code ec;

            if (fs::is_regular_file(path, ec)) {
                row[m_Columns.m_col_size_text] = FileUtils::format_size(fs::file_size(path, ec));
                row[m_Columns.m_col_size_real] = fs::file_size(path, ec);
            } else {
                row[m_Columns.m_col_size_text] = "";
                row[m_Columns.m_col_size_real] = 0;
            }

            auto ftime = fs::last_write_time(path, ec);
            if (!ec) row[m_Columns.m_col_date] = FileUtils::format_date(ftime);

            row[m_Columns.m_col_type] = std::string(FileUtils::get_file_type(path));

            auto* item = Gtk::make_managed<FileIconItem>(icon_display_name, icon, path);
            m_FlowBox.append(*item);

        }
        if (m_current_group_by != GroupBy::None) {
            m_TreeView.expand_all();
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
    }
    set_title(m_current_path.c_str());

    m_BtnUp.set_sensitive(m_current_path != m_current_path.root_path());
}

void FileExplorerWindow::open_or_navigate(const fs::path& path) {
    if (fs::is_directory(path)) {
        navigate_to(path);
    } else {
        try {
            auto file_obj = Gio::File::create_for_path(path.string());
            Gio::AppInfo::launch_default_for_uri(file_obj->get_uri());
        } catch (...) {}
    }
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
        bool is_header = (*iter)[m_Columns.m_col_is_group_header];
        if (is_header) {
            if (m_TreeView.row_expanded(path)) m_TreeView.collapse_row(path);
            else m_TreeView.expand_row(path, false);
            return;
        }
        std::string name = Glib::ustring((*iter)[m_Columns.m_col_name]);
        open_or_navigate(m_current_path / name);
    }
}

void FileExplorerWindow::on_icon_item_activated(Gtk::FlowBoxChild* child) {
    auto* item = dynamic_cast<FileIconItem*>(child->get_child());
    if (item) {
        open_or_navigate(item->get_path());
    }
}

void FileExplorerWindow::sort_file_list(std::vector<fs::path> &entries) const {
    std::sort(entries.begin(), entries.end(), [this](const fs::path& a, const fs::path& b) {
        std::error_code ec;
        const bool is_dir_a = fs::is_directory(a);
        const bool is_dir_b = fs::is_directory(b);

        // if (fs::is_regular_file(a, ec) != fs::is_directory(b, ec)) {
        //     return fs::is_directory(b, ec) > fs::is_directory(b, ec);
        // }

        if (is_dir_a != is_dir_b) {
            return is_dir_a;
        }

        return a.filename().string().c_str() < b.filename().string().c_str();

    });
}

int FileExplorerWindow::on_name_sort(const Gtk::TreeModel::const_iterator &a, const Gtk::TreeModel::const_iterator &b) const {
    const Glib::ustring name_a = (*a)[m_Columns.m_col_name];
    const Glib::ustring name_b = (*b)[m_Columns.m_col_name];
    const Glib::ustring type_a = (*a)[m_Columns.m_col_type];
    const Glib::ustring type_b = (*b)[m_Columns.m_col_type];

    const bool is_dir_a = (type_a == "Directory");
    const bool is_dir_b = (type_b == "Directory");

    const fs::path path_a = m_current_path/name_a.c_str();
    const fs::path path_b = m_current_path/name_b.c_str();
    const bool pinned_a = m_pinned_paths.count(path_a);
    const bool pinned_b = m_pinned_paths.count(path_b);

    int sort_col_id;
    Gtk::SortType sort_order;
    const auto sortable = std::dynamic_pointer_cast<Gtk::TreeSortable>(m_refTreeModel);
    sortable->get_sort_column_id(sort_col_id, sort_order);

    if (pinned_a != pinned_b) {
        if (sort_order == Gtk::SortType::ASCENDING) {
            return pinned_a ? -1 : 1;
        }
        return pinned_a ? 1 : -1;
    }

    if (is_dir_a != is_dir_b) {
        if (sort_order == Gtk::SortType::ASCENDING) {
            return is_dir_a ? -1 : 1;
        }
        return is_dir_a ? 1 : -1;
    }

    bool hidden_a = !name_a.empty() && name_a[0] == '.';
    bool hidden_b = !name_b.empty() && name_b[0] == '.';

    if (hidden_a != hidden_b) {
        return hidden_a ? -1 : 1;
    }

    const Glib::ustring fold_a = name_a.casefold();
    const Glib::ustring fold_b = name_b.casefold();

    if (fold_a < fold_b) return -1;
    if (fold_a > fold_b) return 1;
    return 0;
}

void FileExplorerWindow::save_pins() const {
    std::ofstream ofs(std::string(std::getenv("HOME")) + "/.linxer_pins");
    for (const auto& path : m_pinned_paths) {
        ofs << path.string() << "\n";
    }
}

void FileExplorerWindow::load_pins() {
    std::ifstream ifs(std::string(std::getenv("HOME")) + "/.linxer_pins");
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) m_pinned_paths.insert(fs::path(line));
    }
}