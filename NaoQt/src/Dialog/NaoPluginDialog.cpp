#include "Dialog/NaoPluginDialog.h"

#include <Plugin/NaoPluginManager.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QFile>

void NaoPluginDialog::list(QWidget* parent) {
    QDialog* dialog = new QDialog(parent);

    dialog->setMinimumSize(540, 360);

    dialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    dialog->setWindowTitle("libnao loaded plugins");

    QVBoxLayout* dialog_layout = new QVBoxLayout(dialog);

    QTreeWidget* tree_widget = new QTreeWidget(dialog);

    tree_widget->setColumnCount(4);
    tree_widget->setHeaderLabels({ "Name", "Version", "Author", "Description" });
    tree_widget->setEditTriggers(QTreeView::NoEditTriggers);
    tree_widget->setRootIsDecorated(false);
    tree_widget->setUniformRowHeights(true);
    tree_widget->setAnimated(false);
    tree_widget->setItemsExpandable(false);
    tree_widget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFile template_file(":/NaoQt/Resources/AboutPluginTemplate.html");
    template_file.open(QIODevice::ReadOnly);

    QString author_text_template = template_file.readAll();

    template_file.close();

    NaoVector<NaoPlugin> plugins = PluginManager.loaded();

    enum AuthorRoles {
        AuthorText = Qt::UserRole + 1,
        AutorTextRich
    };

    for (const NaoPlugin& plugin : plugins) {
        QTreeWidgetItem* item = new QTreeWidgetItem(tree_widget);

        item->setText(0, plugin.plugin_name());
        item->setText(1, QString::number(plugin.plugin_version()));
        item->setText(2, plugin.author_name());
        item->setText(3, plugin.plugin_desc());

        item->setData(0, AuthorText, plugin.author_text_plain());
        item->setData(0, AutorTextRich, plugin.author_text_rich());
    }

    tree_widget->resizeColumnToContents(0);
    tree_widget->resizeColumnToContents(1);
    tree_widget->resizeColumnToContents(2);

    connect(tree_widget, &QTreeWidget::itemDoubleClicked, [dialog, author_text_template](QTreeWidgetItem* item) {
        if (item) {
            QDialog* subdialog = new QDialog(dialog);

            subdialog->setWindowFlag(Qt::WindowContextHelpButtonHint, false);

            subdialog->setWindowTitle(item->text(0));

            QVBoxLayout* layout = new QVBoxLayout(subdialog);

            layout->addWidget(new QLabel(author_text_template
                .arg(item->text(2))
                .arg(item->data(0, AutorTextRich).toString())));

            subdialog->setModal(true);

            subdialog->setMinimumWidth(480);

            subdialog->show();
        }
    });

    dialog_layout->addWidget(new QLabel("<h4>Currently loaded plugins for libnao:</h4>", dialog));
    dialog_layout->addWidget(tree_widget);

    dialog->setModal(true);

    connect(dialog, &QDialog::finished, &QDialog::deleteLater);

    dialog->show();
}

