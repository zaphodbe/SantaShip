#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPushButton>
#include <QSignalMapper>

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
    void OnSmaller();
    void OnBigger();
    void OnSelectionChanged(QItemSelection selected,QItemSelection deselected);
    void OnAddPrinter();
    void OnRemovePrinter();
    void OnPrint(int index);
    void on_actionAdd_Printer_triggered(bool checked);
    void on_actionRemove_Printer_triggered(bool checked);

private:
    Ui::MainWindow          *ui;
    QFileSystemModel        *fileModel;
    QItemSelectionModel     *fileSelection;
    QFileThumbnailProvider  *fileThumbnail;
    QList<QPrinter*>         printerList;
    QList<QPushButton*>      printButtonList;
    QGraphicsScene          *graphicsScene;
    QSignalMapper           *signalMapper;
};

#endif // MAINWINDOW_H
