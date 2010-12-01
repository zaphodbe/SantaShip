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
    // Start the ui engine
    ui->setupUi(this);

    // Setup the initial directory
    QString dirName = QDir::homePath() + "/" + DEFAULT_DIR;
    qDebug() << "dirName" << dirName;

    // Setup a list of what files we can process
    QStringList filterList;
    filterList << "*.jpg";
    filterList << "*.bmp";
    filterList << "*.png";
    filterList << "*.gif";
    filterList << "*.jpeg";
    filterList << "*.pbm";
    filterList << "*.tiff";

    // Define the thumbnail provider
    fileThumbnail = new QFileThumbnailProvider();

    // Initialize the model of the file system
    fileModel = new QFileSystemModel(this);
    fileModel->setNameFilters(filterList);
    fileModel->setFilter(QDir::Files);
    fileModel->setNameFilterDisables(false);
    fileModel->setIconProvider(fileThumbnail);
    fileModel->setRootPath(dirName);

    // Initialize the list view to display the file system model
    ui->listView->setModel(fileModel);
    ui->listView->setRootIndex(fileModel->index(dirName));

    // Setup the class to track selections on the list view
    fileSelection = new QItemSelectionModel(fileModel);
    ui->listView->setSelectionModel(fileSelection);

    connect(fileSelection, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnSelectionChanged(QItemSelection,QItemSelection)));

    // Setup the graphics scene to paint the images in
    graphicsScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(graphicsScene);

    // Setup the mapper to map the printer buttons as they are added
    signalMapperPrint = new QSignalMapper(this);
    connect(signalMapperPrint, SIGNAL(mapped(int)), this, SLOT(OnPrint(int)));

    // Setup the mapper to map the layout buttons as they are added
    signalMapperLayout = new QSignalMapper(this);
    connect(signalMapperLayout, SIGNAL(mapped(QString)), this, SLOT(OnLayout(QString)));

    // Setup the Layout buttons.
    // ToDo load these from discription files
    QPushButton *button;
    button = new QPushButton(QString("1 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,0);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));
    layout = button->text(); // Set the default layout.

    button = new QPushButton(QString("2 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,1);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    button = new QPushButton(QString("3 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,2);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    button = new QPushButton(QString("4 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,3);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    button = new QPushButton(QString("8 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,4);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    button = new QPushButton(QString("32 Up"));
    ui->gridLayoutLayoutButtons->addWidget(button,0,5);
    signalMapperLayout->setMapping(button,button->text());
    connect(button, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));
}

MainWindow::~MainWindow()
{
    delete signalMapperLayout;
    delete signalMapperPrint;
    delete graphicsScene;
    delete fileSelection;
    delete fileModel;
    delete fileThumbnail;
    delete ui;
}

/*
 * Normal Event driven slots.
 */

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

void MainWindow::LoadImages()
{
    int imageIndex,layoutIndex;

    qDebug() << __FUNCTION__ << this->layout;
    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Empty the scene
    graphicsScene->clear();

    // For Each image location in the layout place an image.
    // Repeat the image if there are more locations than selected images.
    for (imageIndex = 0, layoutIndex = 0; layoutIndex < layout.toInt(); imageIndex++, layoutIndex++) {

        // Check if we are trying to display more images than selected
        // and loop back to the first image.
        if (imageIndex >= indexList.length()) imageIndex = 0;

        // Load the image into a pixmap
        QPixmap pixmap(fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath());

        // Put the pixmap on the display
        QGraphicsPixmapItem *item = graphicsScene->addPixmap(pixmap);

        // Move/Scale the image to the appropriate location
        //item->setPos(0,0);
        //item->setVisible(true);
    }
}

void MainWindow::OnSelectionChanged(QItemSelection selected,QItemSelection deselected)
{
//    qDebug() << "selected" << selected;
//    qDebug() << "deselected" << deselected;
    LoadImages();
}

void MainWindow::OnLayout(QString layout)
{
//    qDebug() << __FUNCTION__ << layout;
    this->layout = layout; // Set the current layout
    LoadImages();
}

void MainWindow::OnAddPrinter()
{
//    qDebug() << __FUNCTION__;
    QPrinter *printer = new QPrinter();
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        qDebug() << "adding" << printer->printerName() << printer->resolution();
        printerList.append(printer);

        QPushButton *button = new QPushButton(printer->printerName());
        printButtonList.append(button);
        ui->verticalLayoutPrintButtons->addWidget(button);

        signalMapperPrint->setMapping(button,printButtonList.length() - 1);
        connect(button, SIGNAL(clicked()), signalMapperPrint, SLOT(map()));
    }
}

void MainWindow::OnRemovePrinter()
{
    qDebug() << __FUNCTION__;
}

void MainWindow::OnPrint(int index)
{
    qDebug() << __FUNCTION__ << printerList.at(index)->printerName();

    // Do the printing here

    // Set the copies count back to 1
    ui->spinBoxCopies->setValue(1);

    // Set the layout back to default
    this->layout = QString("1 Up");

    // Update the display
    LoadImages();
}

/*
 * Automatically connected action driven slots
 */
void MainWindow::on_actionAdd_Printer_triggered(bool checked)
{
    //qDebug() << __FUNCTION__;
    OnAddPrinter();
}

void MainWindow::on_actionRemove_Printer_triggered(bool checked)
{
    //qDebug() << __FUNCTION__;
    OnRemovePrinter();
}
