/** @file MainWindow.h

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2016</p>
*/

#ifndef SOSRC_MAINWINDOW_H
#define SOSRC_MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    struct Private;
    Private* p_;
};

#endif // SOSRC_MAINWINDOW_H
