/** @file MainWindow.cpp

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2016</p>
*/

#include "MainWindow.h"

struct MainWindow::Private
{
    Private(MainWindow*w)
        : p         (w)
    { }

    MainWindow* p;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow   (parent)
    , p_            (new Private(this))
{
}

MainWindow::~MainWindow()
{
    delete p_;
}
