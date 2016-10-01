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

#ifndef SONOTSRC_MAINWINDOW_H
#define SONOTSRC_MAINWINDOW_H

#include <QMainWindow>

namespace Sonot {

class Score;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString getScoreFilename(bool forSave);
    QString getSynthFilename(bool forSave);

public slots:

    void setScore(const Score&);

protected:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;
private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_MAINWINDOW_H
