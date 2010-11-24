#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>

#ifndef DEFAULT_DIR
    // Expected to be a subdirectory under the home directory
    #define DEFAULT_DIR "Pictures"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString dirName = QDir::homePath() + "/" + DEFAULT_DIR;
    qDebug() << "dirName" << dirName;

    QStringList filterList;
    filterList << "*.jpg";
    filterList << "*.bmp";
    filterList << "*.png";
    filterList << "*.gif";
    filterList << "*.jpeg";
    filterList << "*.pbm";
    filterList << "*.tiff";

    fileThumbnail = new QFileThumbnailProvider();

    fileModel = new QFileSystemModel(this);
    fileModel->setNameFilters(filterList);
    fileModel->setFilter(QDir::Files);
    fileModel->setNameFilterDisables(false);
    fileModel->setIconProvider(fileThumbnail);
    fileModel->setRootPath(dirName);

    ui->listView->setModel(fileModel);
    ui->listView->setRootIndex(fileModel->index(dirName));

    fileSelection = new QItemSelectionModel(fileModel);
    ui->listView->setSelectionModel(fileSelection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,tr("Open Directory"),fileModel->rootPath());

    qDebug() << "OnDir" << directory;
    fileModel->setRootPath(directory);

    ui->listView->setRootIndex(fileModel->index(directory));
}
