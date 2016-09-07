/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <QApplication>
#include "gui/MainWindow.h"

#if 1
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Sonot::MainWindow w;
    w.show();

    return a.exec();
}
#else
#include <QStandardItemModel>
#include <QLayout>
#include <QComboBox>
#include <QListView>
#include <QTableView>
int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QStandardItemModel model(3, 1); // 3 rows, 1 col
    for (int r = 0; r < 3; ++r)
    {
        QStandardItem* item = new QStandardItem(QString("Item %0").arg(r));

        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);

        model.setItem(r, 0, item);
    }

    QComboBox* combo = new QComboBox();
    combo->setModel(&model);

    QListView* list = new QListView();
    list->setModel(&model);

    QTableView* table = new QTableView();
    table->setModel(&model);

    QWidget container;
    QVBoxLayout* containerLayout = new QVBoxLayout();
    container.setLayout(containerLayout);
    containerLayout->addWidget(combo);
    containerLayout->addWidget(list);
    containerLayout->addWidget(table);

    container.show();

    return app.exec();
}
#endif
