/*
    This file is part of NaoQt.

    NaoQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NaoQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with NaoQt.  If not, see <https://www.gnu.org/licenses/>.
*/


#pragma once

#include <QtWidgets/QMainWindow>

#define NAOQT_VERSION_MAJOR 1
#define NAOQT_VERSION_MINOR 0

class QPushButton;
class QLineEdit;
class QTreeWidget;

class NaoQt : public QMainWindow {
    Q_OBJECT

    public:

    // Constructors
    NaoQt(QWidget *parent = nullptr);

    // Static getters
    static QString get_config_path();
    static QString get_text_resource(const QString& path);

    private slots:
    void view_double_click();
    void view_context_menu();
    void view_sort_column();
    void view_up();
    void view_refresh();
    void open_folder();

    void path_display_changed();

    void fsm_notify();

    void about_naoqt();
    void about_libnao();
    void about_icons8();
    void about_qt();
    void libnao_plugins();

    private:
    // Loads all settings from the .ini file
    void _load_settings();
    static void _write_default_settings();

    void _init_window();
    void _load_plugins();
    void _init_filesystem();

    // Private members

    // Stores all settings in key/value pairs
    std::map<QString, QString> _m_settings;

    QPushButton* _m_up_button;
    QPushButton* _m_refresh_button;
    QLineEdit* _m_path_display;
    QPushButton* _m_browse_button;

    QTreeWidget* _m_tree_widget;
};
