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

#include "Dialog/NaoPluginDialog.h"

#include <Logging/NaoLogging.h>
#include <Plugin/NaoPluginManager.h>

#include <QSettings>
#include <QCoreApplication>

#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeWidget>
#include <QHeaderView>
#include <QFileIconProvider>
#include <QMenuBar>

//// Public

// Constructors

NaoQt::NaoQt(QWidget *parent)
    : QMainWindow(parent) {

    _load_settings();

    _init_window();

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

void NaoQt::_init_window() {
    setMinimumSize(540, 360);
    resize(960, 640);
    
    QWidget* central_widget = new QWidget(this);
    QVBoxLayout* central_layout = new QVBoxLayout(central_widget);
    QHBoxLayout* path_display_layout = new QHBoxLayout();

    _m_refresh_button = new QPushButton(QIcon(":/NaoQt/Resources/refresh.png"), "", central_widget);
    _m_path_display = new QLineEdit(this);
    _m_browse_button = new QPushButton("Browse", central_widget);

    _m_tree_widget = new QTreeWidget(central_widget);
    _m_tree_widget->setColumnCount(4);
    _m_tree_widget->setHeaderLabels({ "Name", "Size", "Type", "Compressed" });
    _m_tree_widget->setEditTriggers(QTreeView::NoEditTriggers);
    _m_tree_widget->setRootIsDecorated(false);
    _m_tree_widget->setContextMenuPolicy(Qt::CustomContextMenu);
    _m_tree_widget->setUniformRowHeights(true);
    _m_tree_widget->setAnimated(false);
    _m_tree_widget->setItemsExpandable(false);
    _m_tree_widget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(_m_tree_widget, &QTreeWidget::itemDoubleClicked, this, &NaoQt::view_double_click);
    connect(_m_tree_widget, &QTreeView::customContextMenuRequested, this, &NaoQt::view_context_menu);

    QHeaderView* header_view = _m_tree_widget->header();

    header_view->setSectionsClickable(true);
    header_view->setSortIndicatorShown(true);
    header_view->setSortIndicator(0, Qt::DescendingOrder);

    connect(header_view, &QHeaderView::sortIndicatorChanged, this, &NaoQt::view_sort_column);

    _m_browse_button->setMaximumHeight(22);
    _m_browse_button->setIcon(QFileIconProvider().icon(QFileIconProvider::Folder));

    connect(_m_refresh_button, &QPushButton::released, this, &NaoQt::view_refresh);
    connect(_m_browse_button, &QPushButton::released, this, &NaoQt::open_folder);
    connect(_m_path_display, &QLineEdit::editingFinished, this, &NaoQt::path_display_changed);

    path_display_layout->addWidget(_m_refresh_button);
    path_display_layout->addWidget(_m_path_display);
    path_display_layout->addWidget(_m_browse_button);
    path_display_layout->setContentsMargins(4, 4, 4, 0);

    central_layout->setContentsMargins(0, 0, 0, 0);
    central_layout->addLayout(path_display_layout);
    central_layout->addWidget(_m_tree_widget);

    _m_tree_widget->setStyleSheet("QTreeView { border: none; }");

    setCentralWidget(central_widget);

    _m_tree_widget->setFocus();



    QMenuBar* menu = new QMenuBar(this);

    QMenu* file_menu = new QMenu("File", menu);
    QAction* open_folder = new QAction("Open folder", file_menu);
    QAction* exit_app = new QAction("Exit", file_menu);

    connect(open_folder, &QAction::triggered, this, &NaoQt::open_folder);
    connect(exit_app, &QAction::triggered, this, &QMainWindow::close);

    file_menu->addAction(open_folder);
    file_menu->addSeparator();
    file_menu->addAction(exit_app);

    QMenu* help_menu = new QMenu("Help", menu);
    QAction* about_naoqt = new QAction("About NaoQt", help_menu);
    QAction* about_libnao = new QAction("About libnao", help_menu);
    QAction* about_qt = new QAction("About Qt", help_menu);
    QAction* libnao_plugins = new QAction("Plugins", help_menu);

    connect(about_naoqt, &QAction::triggered, this, &NaoQt::about_naoqt);
    connect(about_libnao, &QAction::triggered, this, &NaoQt::about_libnao);
    connect(about_qt, &QAction::triggered, this, &NaoQt::about_qt);
    connect(libnao_plugins, &QAction::triggered, this, &NaoQt::libnao_plugins);

    help_menu->addAction(about_naoqt);
    help_menu->addAction(about_libnao);
    help_menu->addAction(about_qt);
    help_menu->addSeparator();
    help_menu->addAction(libnao_plugins);

    menu->addMenu(file_menu);
    menu->addMenu(help_menu);

    setMenuBar(menu);

}

void NaoQt::_load_plugins() {
    bool success = PluginManager.init(_m_settings.at("plugins/plugins_directory").toStdString().c_str());

    if (!success) {

        QString msg = "Plugins failed to load:\n";

        NaoVector<NaoPair<NaoString, NaoString>> errs = PluginManager.errored_list();
        for (const NaoPair<NaoString, NaoString>& err : errs) {
            msg.append(QString("%0: %1\n").arg(err.first.c_str(), err.second.c_str()));
        }

        QMessageBox::critical(
            this,
            "Plugins failed to load",
            msg);
    }
}


//// Slots

void NaoQt::view_double_click() {
    
}

void NaoQt::view_context_menu() {
    
}

void NaoQt::view_sort_column() {
    
}

void NaoQt::view_refresh() {
    
}

void NaoQt::open_folder() {
    
}

void NaoQt::path_display_changed() {
    
}

void NaoQt::about_naoqt() {
    QFile about_file(":/NaoQt/Resources/AboutNaoQt.html");
    about_file.open(QIODevice::ReadOnly | QIODevice::Text);

    QString about_naoqt = about_file.readAll();

    about_file.close();

    QMessageBox::about(this, "About NaoQt",
        about_naoqt.arg(QString("version %0.%1").arg(NAOQT_VERSION_MAJOR).arg(NAOQT_VERSION_MINOR)));
}

void NaoQt::about_libnao() {
    QFile about_file(":/NaoQt/Resources/Aboutlibnao.html");
    about_file.open(QIODevice::ReadOnly | QIODevice::Text);

    QString about_libnao = about_file.readAll();

    about_file.close();

    QMessageBox::about(this, "About libnao",
        about_libnao.arg(QString("version %0.%1").arg(LIBNAO_VERSION_MAJOR).arg(LIBNAO_VERSION_MINOR)));
}

void NaoQt::about_qt() {
    QMessageBox::aboutQt(this, "About Qt");
}

void NaoQt::libnao_plugins() {
    NaoPluginDialog::list(this);
}
