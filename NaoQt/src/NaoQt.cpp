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

#define N_LOG_ID "NaoQt"
#include <Logging/NaoLogging.h>

#include <Plugin/NaoPluginManager.h>
#include <Filesystem/NaoFileSystemManager.h>
#include <Filesystem/Filesystem.h>
#include <Filesystem/NTreeNode.h>
#include <Utils/SteamUtils.h>
#include <Utils/DesktopUtils.h>
#include <UI/NaoUIManager.h>
#include <IO/NaoIO.h>

#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeWidget>
#include <QHeaderView>
#include <QFileIconProvider>
#include <QMenuBar>
#include <QtConcurrent>
#include <QFileDialog>
#include <QMetaType>

#ifdef N_WINDOWS
#   include <shellapi.h>
#endif

Q_DECLARE_METATYPE(NTreeNode*);

//// Public

// Constructors

NaoQt::NaoQt(QWidget *parent)
    : QMainWindow(parent)
    , _is_moving(false) {

    try {
        // First load settings for configuration
        _load_settings();

        // Set log output file
        NaoLogging::set_log_file(_m_settings.at("logging/log_file"));

        nlog << "Loaded settings";

        // Create window
        _init_window();
        nlog << "Initialised window";

        // Load all plugins
        _load_plugins();
        nlog << "Loaded plugins";

        // Initialise filesystem
        _init_filesystem();
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, "Initialisation error", e.what());

        QTimer::singleShot(0, this, &QMainWindow::close);
    }
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
    // If the settings file doesn't exist, create a new one
    if (!QFile(get_config_path()).exists()) {
        _write_default_settings();
    }

    // Load the settings
    QSettings settings(get_config_path(), QSettings::IniFormat);
    QStringList existings_keys = settings.allKeys();

    for (const std::pair<const char*, const char*> pair : DefaultSettings) {
        // If a key doesn't exist
        if (!existings_keys.contains(pair.first)) {
            // Use it's default value
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

    NaoUI.set_hwnd(reinterpret_cast<HWND>(winId()));
}

void NaoQt::_load_plugins() {
    if (!NPM.init(_m_settings.at("plugins/plugins_directory"))) {
        throw std::runtime_error("Plugins failed to load.");
    }
}

void NaoQt::_init_filesystem() {

    const NaoString default_path = SteamUtils::game_path(_m_settings.at("filesystem/game"),
        _m_settings.at("filesystem/subdir"),
        _m_settings.at("filesystem/fallback"));

    auto future_watcher = new QFutureWatcher<bool>(this);

    connect(future_watcher, &QFutureWatcher<bool>::finished, this, [future_watcher, this] {
        
        if (future_watcher->isCanceled() || !future_watcher->result()) {
            QMessageBox::critical(this, "NFSM::init", "Got invalid result");

            close();
        }

        nlog << "Initialised filesystem";

        fsm_object_changed();
    });
    connect(future_watcher, &QFutureWatcher<bool>::finished, &QFutureWatcher<bool>::deleteLater);

    future_watcher->setFuture(QtConcurrent::run(&NFSM, &NaoFileSystemManager::init, default_path));
}

void NaoQt::_move_async(const QString& to, bool _refresh) {
    if (_is_moving) {
        nerr << "Already moving";
        return;
    }

    if (!_refresh) {
        if (to.endsWith(NFSM.current()->name())) {
            return;
        }
    }

    _is_moving = true;

    auto future_watcher = new QFutureWatcher<bool>(this);

    (void) connect(future_watcher, &QFutureWatcher<bool>::finished, this, [future_watcher, this] {
        if (future_watcher->isCanceled() || !future_watcher->result()) {
            QMessageBox::critical(this, "NaoFSM::move", "Move error");
        } else {
            fsm_object_changed();
        }

        _is_moving = false;
    });

    (void) connect(future_watcher, &QFutureWatcher<bool>::finished, &QFutureWatcher<bool>::deleteLater);

    future_watcher->setFuture(QtConcurrent::run(&NFSM, &NaoFileSystemManager::move, to));
}

//// Slots

void NaoQt::view_double_click(QTreeWidgetItem* item, int col) {
    Q_UNUSED(col);

    NTreeNode* node = item->data(0, NodeRole).value<NTreeNode*>();

    if (node->is_dir()) {
        _move_async(node->path());
    }

    /*if (object->is_dir()) {
        _move_async(object->name());
        return;
    }

    NaoPlugin* plugin = PluginManager.enter_plugin(object);

    if (!plugin && QFile(object->name()).exists()) {
        DesktopUtils::open_file(object->name());
    } else if (plugin) {
        _move_async(object->name());
    }*/
}

void NaoQt::view_context_menu(const QPoint& pos) {

#define ADDOPT(text, target, handler) { \
    QAction* act = new QAction(text, menu); \
    connect(act, &QAction::triggered, target, handler); \
    menu->addAction(act); \
}

    nlog << "Constructing context menu";

    QMenu* menu = new QMenu(this);
    connect(menu, &QMenu::triggered, menu, &QMenu::deleteLater);

    QTreeWidgetItem* item = _m_tree_widget->itemAt(pos);

    if (!item) {
        nlog << "No item";

        if (QDir(_m_path_display->text()).exists()) {
            ADDOPT("Refresh view", this, &NaoQt::view_refresh);

            ADDOPT("Show in explorer", this, [this] {
                if (QDir(_m_path_display->text()).exists()) {
                    DesktopUtils::open_in_explorer(_m_path_display->text() + N_PATHSEP);
                }
            });
        }

    } else {
        /*NaoObject* object = item->data(0, ObjectRole).value<NaoObject*>();

        nlog << "Object name:" << object->name();

        bool append_show = false;
        NaoPlugin* target_plugin = nullptr;
        NaoPlugin* current_plugin = NaoFSM.current_plugin();
        if ((target_plugin = PluginManager.context_menu_plugin(object))) {
            append_show = true;

            ADDOPT("Open", this, ([this, object] {
                _move_async(object->name());
            }));
        } else if (QFile(object->name()).exists()) {
            append_show = true;

            ADDOPT("Open", this, [object] {
                DesktopUtils::open_file(object->name());
            });
            ADDOPT("Open with...", this, [object] {
                DesktopUtils::open_file(object->name(), true);
            });
        }

        if (target_plugin
            && target_plugin != current_plugin
            && target_plugin->HasContextMenu(object)) {

            nlog << NaoString("Using plugin \"" + target_plugin->DisplayName() + '"');

            menu->addSection(target_plugin->DisplayName());

            const int64_t count = menu->actions().size();

            for (NaoAction* action : target_plugin->ContextMenu(object)) {

                QAction* act = new QAction(action->ActionName(), menu);
                connect(act, &QAction::triggered, this, [this, action, object] {

                    auto watcher = new QFutureWatcher<bool>(this);
                    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, action, watcher] {
                        if (!watcher->result()) {
                            QMessageBox::warning(this, "Action failed",
                                "Failed executing action with name: " + action->ActionName());
                        }

                        delete action;
                    });
                    connect(watcher, &QFutureWatcher<bool>::finished, &QFutureWatcher<bool>::deleteLater);

                    watcher->setFuture(QtConcurrent::run(action, &NaoAction::Execute, object));
                });

                menu->addAction(act);
            }

            nlog << "Added" << (menu->actions().size() - count) << "action(s)";
        }

        if (current_plugin->HasContextMenu(object)) {

            nlog << NaoString("Using source plugin \""
                + current_plugin->DisplayName() + '"');

            menu->addSection(current_plugin->DisplayName());

            const int64_t count = menu->actions().size();

            for (NaoAction* action : current_plugin->ContextMenu(object)) {

                QAction* act = new QAction(action->ActionName(), menu);
                connect(act, &QAction::triggered, this, [this, action, object] {
                    if (!action->Execute(object)) {
                        QMessageBox::warning(this, "Action failed",
                            "Failed executing action \"" + action->ActionName() + "\"");
                    }
                });

                menu->addAction(act);
            }

            nlog << "Added" << (menu->actions().size() - count) << "action(s)";
        }

        if (append_show) {

            nlog << "Appending \"Show in explorer\" action";

            menu->addSeparator();

            ADDOPT("Show in explorer", this, [object] {
                DesktopUtils::show_in_explorer(object->name());
            });
        }*/
    }

    if (menu->isEmpty()) {
        menu->addAction("No available actions")->setDisabled(true);
    }

    menu->popup(_m_tree_widget->viewport()->mapToGlobal(pos));

#undef ADDOPT
}

void NaoQt::view_sort_column(int index, Qt::SortOrder order) {

    NaoVector<QTreeWidgetItem*> files;
    NaoVector<QTreeWidgetItem*> directories;

    while (_m_tree_widget->topLevelItemCount() > 0) {
        QTreeWidgetItem* row = _m_tree_widget->takeTopLevelItem(0);

        (row->data(0, IsDirectoryRole).toBool() ? directories : files).push_back(row);
    }

    // Always sort directories alphabetically
    std::sort(std::begin(directories), std::end(directories),
        [](QTreeWidgetItem* a, QTreeWidgetItem* b) -> bool {
        return a->text(0).compare(b->text(0), Qt::CaseInsensitive) < 0;
    });

    std::sort(std::begin(files), std::end(files),
        [index](QTreeWidgetItem* a, QTreeWidgetItem* b) -> bool {

        if (index == 1) {
            const qint64 size_a = a->data(1, ItemSizeRole).toLongLong();
            const qint64 size_b = b->data(1, ItemSizeRole).toLongLong();

            if (size_a != size_b) {
                return size_a > size_b;
            }
        } else if(index == 2) {
            const QString type_a = a->text(2);
            const QString type_b = b->text(2);

            if (type_a != type_b) {
                return type_a.compare(type_b, Qt::CaseInsensitive) < 0;
            }
        } else if (index == 3) {
            const double ratio_a = a->data(3, CompressionRatioRole).toDouble();
            const double ratio_b = b->data(3, CompressionRatioRole).toDouble();

            if (ratio_a != ratio_b) {
                return ratio_a > ratio_b;
            }
        }

       return a->text(0).compare(b->text(0), Qt::CaseInsensitive) < 0;
    });

    if (order == Qt::AscendingOrder) {
        for (int64_t i = int64_t(std::size(directories)) - 1; i >= 0; --i) {
            _m_tree_widget->addTopLevelItem(directories.at(i));
        }

        for (int64_t i = int64_t(std::size(files)) - 1; i >= 0; --i) {
            _m_tree_widget->addTopLevelItem(files.at(i));
        }
    } else {
        for (size_t i = 0; i < std::size(directories); ++i) {
            _m_tree_widget->addTopLevelItem(directories.at(i));
        }

        for (size_t i = 0; i < std::size(files); ++i) {
            _m_tree_widget->addTopLevelItem(files.at(i));
        }
    }
}

void NaoQt::view_up() {
    _move_async(NFSM.current()->parent()->path());
}

void NaoQt::view_refresh() {
    _move_async(NFSM.current()->path(), true);
}

void NaoQt::open_folder() {
    QString target = QFileDialog::getExistingDirectory(this, "Open folder",
        NFSM.current()->path());

    if (!target.isNull() && !target.isEmpty()) {
        _move_async(target);
    }
}

void NaoQt::path_display_changed() {
    NaoString new_path = _m_path_display->text();

    if (new_path.contains(N_PATHSEP)) {
        bool success = true;

        NaoString next_path;

        while (!QDir(new_path).exists()) {
            next_path = new_path.substr(0, new_path.last_index_of(N_PATHSEP));

            if (new_path == next_path) {
                success = false;
                break;
            }

            new_path = next_path;
        }

        if (success) {
            _move_async(new_path);
        }
    }

    //_m_path_display->setText(NaoFSM.current_path());
}

void NaoQt::fsm_object_changed() {
    // Retrieve current node
    NTreeNode* current_object = NFSM.current();

    // Disable buttons as needed
    NTreeNode* parent = current_object->parent();
    if (!parent) {
        _m_up_button->setEnabled(false);
    } else {
        _m_up_button->setEnabled(true);
    }

    // Clear view
    _m_tree_widget->clear();

    // Set path display to path of current object
    _m_path_display->setText(current_object->path());

    // Static file provider
    static QFileIconProvider ficonprovider;

    // Iterate over all children
    for (NTreeNode* const& child : current_object->children()) {
        // New item
        QTreeWidgetItem* item = new QTreeWidgetItem(_m_tree_widget);

        // Display name
        item->setText(0, child->display_name());

        // Description
        item->setText(2, NFSM.description(child));

        // If it's a directory
        item->setData(0, IsDirectoryRole, child->is_dir());

        // Set the node
        item->setData(0, NodeRole, QVariant::fromValue(child));

        // Icon based on if it's a directory or not
        if (child->is_dir()) {
            // Folder icon
            item->setIcon(0, ficonprovider.icon(QFileIconProvider::Folder));

            // No size
            item->setData(1, ItemSizeRole, -1i64);
        } else {
            // Icon from existing file
            item->setIcon(0, ficonprovider.icon(QFileInfo(child->path())));

            // Store IO
            NaoIO* io = child->io();

            // Set real size attributes
            item->setText(1, NaoString::bytes(io->size()));
            item->setData(1, ItemSizeRole, io->size());

            // If no description was set
            if (item->text(2).isEmpty()) {
                // Try to retrieve it
                static QMimeDatabase db;
                item->setText(2, db.mimeTypeForUrl(
                    QUrl::fromLocalFile(child->name())).comment());
            }

            // Compression ratio
            double ratio = io->size() / double(io->virtual_size());
            item->setData(3, CompressionRatioRole, ratio);

            // If the file doesn't exist it can be compressed
            if (!QFile(child->path()).exists()) {
                item->setText(3, QString("%0%").arg(qRound(100. * ratio)));
            }
        }

        // Add item
        _m_tree_widget->addTopLevelItem(item);
    }

    // Resize all columns
    for (int i = 0; i < _m_tree_widget->columnCount(); ++i) {
        _m_tree_widget->resizeColumnToContents(i);
    }

    _m_tree_widget->header()->resizeSection(0, _m_tree_widget->width() / 4);

    // Re-sort
    view_sort_column(_m_tree_widget->header()->sortIndicatorSection(), _m_tree_widget->header()->sortIndicatorOrder());
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
