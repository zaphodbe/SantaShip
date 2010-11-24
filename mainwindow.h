#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>

#include "filethumbnailprovider.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

public slots:
    void OnDir();

private:
    Ui::MainWindow          *ui;
    QFileSystemModel        *fileModel;
    QItemSelectionModel     *fileSelection;
    QFileThumbnailProvider  *fileThumbnail;
};

#endif // MAINWINDOW_H
