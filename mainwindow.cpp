#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsItem>

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

    connect(fileSelection, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnSelectionChanged(QItemSelection,QItemSelection)));

    graphicsScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(graphicsScene);

    signalMapper = new QSignalMapper(this);
}

MainWindow::~MainWindow()
{
    delete signalMapper;
    delete graphicsScene;
    delete fileSelection;
    delete fileModel;
    delete fileThumbnail;
    delete ui;
}

void MainWindow::OnDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,tr("Open Directory"),fileModel->rootPath());

    qDebug() << "OnDir" << directory;
    fileModel->setRootPath(directory);

    ui->listView->setRootIndex(fileModel->index(directory));
}

void MainWindow::OnSmaller()
{
    QSize size = ui->listView->iconSize();

    size.setWidth(size.width() / 2);
    size.setHeight(size.width() * 3 / 5);

    ui->listView->setIconSize(size);

    ui->pushButtonBigger->setEnabled(true);
    if (size.width() <= 125) {
        ui->pushButtonSmaller->setEnabled(false);
    }
}

void MainWindow::OnBigger()
{
    QSize size = ui->listView->iconSize();

    size.setWidth(size.width() * 2);
    size.setHeight(size.width() * 3 / 5);

    ui->listView->setIconSize(size);

    ui->pushButtonSmaller->setEnabled(true);
    if (size.width() >= 500) {
        ui->pushButtonBigger->setEnabled(false);
    }
}

void MainWindow::OnSelectionChanged(QItemSelection selected,QItemSelection deselected)
{
    int i;
    //qDebug() << "selected" << selected;
    //qDebug() << "deselected" << deselected;
    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Empty the scene
    graphicsScene->clear();
    //graphicsScene->addText(tr("Hello World!"));

    // Add each selected image
    for (i = 0; i < indexList.length(); i++) {
        qDebug() << "selection" << i << fileModel->fileInfo(indexList.at(i)).absoluteFilePath();
        QPixmap pixmap(fileModel->fileInfo(indexList.at(i)).absoluteFilePath());
        QGraphicsPixmapItem *item = graphicsScene->addPixmap(pixmap);
        //item->setPos(0,0);
        //item->setVisible(true);
    }
}

void MainWindow::OnAddPrinter()
{
    //qDebug() << __FUNCTION__;
    QPrinter *printer = new QPrinter();
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        qDebug() << "added" << printer->printerName() << printer->resolution();
        printerList.append(printer);

        QPushButton *button = new QPushButton(printer->printerName());
        printButtonList.append(button);
        ui->verticalLayoutPrintButtons->addWidget(button);

        //signalMapper->setMapping(button,);
        //connect(button, SIGNAL())
    }
}

void MainWindow::on_actionAdd_Printer_triggered(bool checked)
{
    //qDebug() << __FUNCTION__;
    OnAddPrinter();
}

void MainWindow::on_actionRemove_Printer_triggered(bool checked)
{
    //qDebug() << __FUNCTION__;
}
