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

#include "NaoQt.h"

#include "DefaultSettingsValues.h"

#include <Logging/NaoLogging.h>
#include <Plugin/NaoPluginManager.h>

#include <QSettings>
#include <QCoreApplication>
#include <QFile>

//// Public

// Constructors

NaoQt::NaoQt(QWidget *parent)
    : QMainWindow(parent) {

    _load_settings();

    _load_plugins();

}

QString NaoQt::get_config_path() {
    return QCoreApplication::applicationDirPath() + "/NaoQt.ini";
}

//// Private

void NaoQt::_load_settings() {
    if (!QFile(get_config_path()).exists()) {
        _write_default_settings();
    }

    QSettings settings(get_config_path(), QSettings::IniFormat);
    QStringList existings_keys = settings.allKeys();

    for (std::pair<const char*, const char*> pair : DefaultSettings) {
        if (!existings_keys.contains(pair.first)) {
            settings.setValue(pair.first, pair.second);
        }

        _m_settings.insert(pair);
    }
}

void NaoQt::_write_default_settings() {
    QSettings settings(get_config_path(), QSettings::IniFormat);

    for (std::pair<const char*, const char*> pair : DefaultSettings) {
        settings.setValue(pair.first, pair.second);
    }
}

void NaoQt::_load_plugins() {
    bool success = PluginManager.init(_m_settings.at("plugins/plugins_directory").toStdString().c_str());
    if (!success) {

        nDebug << "Errored plugins:";

        /*NaoMap<NaoString, NaoString> errs = PluginManager.errored_list();
        for (NaoMap<NaoString, NaoString>::iterator a = std::begin(errs);
            a != std::end(errs); ++a) {
            nDebug << a->first.c_str() << a->second.c_str();
        }*/
    }
}

