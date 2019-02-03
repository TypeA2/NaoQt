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
#include <Filesystem/NaoFileSystemManager.h>
#include <Filesystem/Filesystem.h>
#include <Utils/SteamUtils.h>

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
#include <QFutureWatcher>

#include <QtConcurrent>

//// Public

// Constructors

NaoQt::NaoQt(QWidget *parent)
    : QMainWindow(parent) {

    _load_settings();

    _init_window();

    _load_plugins();

    _init_filesystem();

}

QString NaoQt::get_config_path() {
    return QCoreApplication::applicationDirPath() + "/NaoQt.ini";
}

QString NaoQt::get_text_resource(const QString& path) {
    QFile file(":/NaoQt/Resources/" + path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QString str = file.readAll();

    file.close();

    return str;
}

//// Private

void NaoQt::_load_settings() {
    if (!QFile(get_config_path()).exists()) {
        _write_default_settings();
    }

    QSettings settings(get_config_path(), QSettings::IniFormat);
    QStringList existings_keys = settings.allKeys();

    for (const std::pair<const char*, const char*> pair : DefaultSettings) {
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

#pragma region Widgets

    QWidget* central_widget = new QWidget(this);
    QVBoxLayout* central_layout = new QVBoxLayout(central_widget);
    QHBoxLayout* path_display_layout = new QHBoxLayout();

    _m_up_button = new QPushButton(QIcon(":/NaoQt/Resources/icons/up.png"), "", central_widget);
    _m_up_button->setToolTip("Go up");
    
    _m_refresh_button = new QPushButton(QIcon(":/NaoQt/Resources/icons/refresh.svg"), "", central_widget);
    _m_refresh_button->setToolTip("Refresh view");

    _m_path_display = new QLineEdit(this);
    _m_browse_button = new QPushButton("Browse", central_widget);
    _m_browse_button->setToolTip("Browse folder");

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

    connect(_m_up_button, &QPushButton::released, this, &NaoQt::view_up);
    connect(_m_refresh_button, &QPushButton::released, this, &NaoQt::view_refresh);
    connect(_m_browse_button, &QPushButton::released, this, &NaoQt::open_folder);
    connect(_m_path_display, &QLineEdit::editingFinished, this, &NaoQt::path_display_changed);

    path_display_layout->addWidget(_m_up_button);
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

#pragma endregion Widgets

#pragma region MenuBar

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
    QAction* about_icons8 = new QAction("About Icons8", help_menu);
    QAction* about_qt = new QAction("About Qt", help_menu);
    QAction* libnao_plugins = new QAction("Plugins", help_menu);

    connect(about_naoqt, &QAction::triggered, this, &NaoQt::about_naoqt);
    connect(about_libnao, &QAction::triggered, this, &NaoQt::about_libnao);
    connect(about_icons8, &QAction::triggered, this, &NaoQt::about_icons8);
    connect(about_qt, &QAction::triggered, this, &NaoQt::about_qt);
    connect(libnao_plugins, &QAction::triggered, this, &NaoQt::libnao_plugins);

    help_menu->addAction(about_naoqt);
    help_menu->addAction(about_libnao);
    help_menu->addAction(about_icons8);
    help_menu->addAction(about_qt);
    help_menu->addSeparator();
    help_menu->addAction(libnao_plugins);

    menu->addMenu(file_menu);
    menu->addMenu(help_menu);

    setMenuBar(menu);

#pragma endregion MenuBar

}

void NaoQt::_load_plugins() {
    bool success = PluginManager.init(_m_settings.at("plugins/plugins_directory"));

    if (!success) {

        QString msg = "Plugins failed to load:\n";

        NaoVector<NaoPair<NaoString, NaoString>> errs = PluginManager.errored_list();
        for (const NaoPair<NaoString, NaoString>& err : errs) {
            msg.append(QString("%0: %1\n").arg(err.first, err.second));
        }

        QMessageBox::critical(
            this,
            "Plugins failed to load",
            msg);
    }
}

void NaoQt::_init_filesystem() {

    const NaoString default_path = SteamUtils::game_path(_m_settings.at("filesystem/default_game"),
        _m_settings.at("filesystem/default_fallback"));

    auto future_watcher = new QFutureWatcher<bool>(this);

    connect(future_watcher, &QFutureWatcher<void>::finished, this, [future_watcher, this] {

        if (!future_watcher->result()) {
            QMessageBox::critical(this, "NaoFSM::init error",
                NaoFSM.last_error());

            throw std::exception(NaoFSM.last_error());
        }

        fsm_object_changed();
    });
    connect(future_watcher, &QFutureWatcher<void>::finished, &QFutureWatcher<void>::deleteLater);

    future_watcher->setFuture(QtConcurrent::run(&NaoFSM, &NaoFileSystemManager::init, default_path));
}

//// Slots

void NaoQt::view_double_click() {
    
}

void NaoQt::view_context_menu() {
    
}

void NaoQt::view_sort_column() {
    
}

void NaoQt::view_up() {

}

void NaoQt::view_refresh() {
    
}

void NaoQt::open_folder() {
    
}

void NaoQt::path_display_changed() {
    
}

void NaoQt::fsm_object_changed() {
    NaoObject* current_object = NaoFSM.current_object();

    _m_path_display->setText(current_object->name());

    static QFileIconProvider ficonprovider;

    const NaoString base_path = current_object->name().copy().append('/').normalize_path();

    for (NaoObject* child : current_object->children()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(_m_tree_widget);

        NaoString name = child->name().copy().normalize_path();

        if (name.starts_with(base_path)) {
            name = name.substr(std::size(base_path));
        }

        item->setText(0, name);

        item->setText(2, child->description());

        if (child->is_dir()) {
            item->setIcon(0, ficonprovider.icon(QFileIconProvider::Folder));

            item->setData(1, ItemSizeRole, -1i64);
        } else {
            const NaoObject::File& file = child->file_ref();

            item->setIcon(0, ficonprovider.icon(QFileInfo(child->name())));

            item->setData(1, ItemSizeRole, file.real_size);

            if (item->text(2).isEmpty()) {
                static QMimeDatabase db;
                item->setText(2, db.mimeTypeForUrl(QUrl::fromLocalFile(child->name())).comment());
            }

            double ratio = file.binary_size / double(file.real_size);
            item->setData(3, CompressionRatioRole, ratio);
            item->setText(3, QString("%0%").arg(qRound(100. * ratio)));
        }
        
        _m_tree_widget->addTopLevelItem(item);
    }

    for (int i = 0; i < _m_tree_widget->columnCount(); ++i) {
        _m_tree_widget->resizeColumnToContents(i);
    }
}


#pragma region About

void NaoQt::about_naoqt() {
    QMessageBox::about(this, "About NaoQt",
        get_text_resource("AboutNaoQt.html").arg(QString("version %0.%1")
            .arg(NAOQT_VERSION_MAJOR).arg(NAOQT_VERSION_MINOR)));
}

void NaoQt::about_libnao() {
    QMessageBox::about(this, "About libnao",
        get_text_resource("Aboutlibnao.html").arg(QString("version %0.%1")
            .arg(LIBNAO_VERSION_MAJOR).arg(LIBNAO_VERSION_MINOR)));
}

void NaoQt::about_icons8() {
    QMessageBox::about(this, "About Icons8",
        get_text_resource("AboutIcons8.html"));
}

void NaoQt::about_qt() {
    QMessageBox::aboutQt(this, "About Qt");
}

void NaoQt::libnao_plugins() {
    NaoPluginDialog::list(this);
}

#pragma endregion
