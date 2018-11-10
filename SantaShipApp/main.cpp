#include <QtWidgets/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);
    MainWindow *w = MainWindow::getInstance();
    w->show();

    return a.exec();
}
