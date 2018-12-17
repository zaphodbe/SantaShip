#include <QtWidgets/QApplication>
#include "mainwindow.h"
#include "SantaLogger.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    // Redirect output to SantaLogger ASAP
    SantaLogger::instance();

    QApplication a(argc, argv);
    MainWindow *w = MainWindow::getInstance();
    w->show();

    return a.exec();
}
