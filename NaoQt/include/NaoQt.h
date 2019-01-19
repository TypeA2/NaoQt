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

class NaoQt : public QMainWindow {
    Q_OBJECT

    public:

    // Constructors
    NaoQt(QWidget *parent = nullptr);

    // Static getters
    static QString get_config_path();

    private:
    // Stores all settings in key/value pairs
    std::map<QString, QString> _m_settings;

    // Loads all settings from the .ini file
    void _load_settings();
    static void _write_default_settings();

    void _load_plugins();
};
