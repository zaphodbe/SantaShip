#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QPrintPreviewDialog>

#ifndef DEFAULT_DIR
    // Expected to be a subdirectory under the home directory
    #define DEFAULT_DIR "Pictures"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    saveSettingsOnExit(0)
{
    // Initialize so we can access the settings
    settings = new QSettings(QString("SantaShip"),QString("SantaShip"));

    // Start the ui engine
    ui->setupUi(this);

    // Setup the right click actions that we will add to the buttons
    actionDeletePictures = new QAction(tr("Delete Selected"),this);
    connect(actionDeletePictures, SIGNAL(triggered()), this, SLOT(OnDeletePictures()));

    signalMapperPrinterRemove = new QSignalMapper(this);
    connect(signalMapperPrinterRemove, SIGNAL(mapped(int)), this, SLOT(OnPrinterRemove(int)));

    signalMapperPrinterSettings = new QSignalMapper(this);
    connect(signalMapperPrinterSettings, SIGNAL(mapped(int)), this, SLOT(OnPrinterSettings(int)));

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
    fileModel->sort(3);
    connect(fileModel, SIGNAL(directoryLoaded(QString)), this, SLOT(OnDirLoaded(QString)));

    // Initialize the list view to display the file system model
    ui->listView->setModel(fileModel);
    ui->listView->setRootIndex(fileModel->index(dirName));
    ui->listView->addAction(actionDeletePictures);
    ui->listView->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Setup the class to track selections on the list view
    fileSelection = new QItemSelectionModel(fileModel);
    ui->listView->setSelectionModel(fileSelection);

    connect(fileSelection, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnSelectionChanged(QItemSelection,QItemSelection)));

    // Setup the graphics scene to paint the images in
    graphicsScene = new QGraphicsScene(this);

    // Setup the mapper to map the printer buttons as they are added
    signalMapperPrint = new QSignalMapper(this);
    connect(signalMapperPrint, SIGNAL(mapped(int)), this, SLOT(OnPrint(int)));

    // Setup the mapper to map the layout buttons as they are added
    signalMapperLayout = new QSignalMapper(this);
    connect(signalMapperLayout, SIGNAL(mapped(QWidget*)), this, SLOT(OnLayout(QWidget*)));

    // Setup the Layout buttons.
    // ToDo load these from discription files
    QImageLayoutButton *layoutButton;

    layoutButton = newImageLayout(QString("1 Up"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,11000));

    layoutButton = newImageLayout(QString("2 Up"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,5500));
    layoutButton->addImage(QRectF(0,5500,8500,5500));

    layoutButton = newImageLayout(QString("4 Up"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,4250,5500));
    layoutButton->addImage(QRectF(4250,0,4250,5500));
    layoutButton->addImage(QRectF(0,5500,4250,5500));
    layoutButton->addImage(QRectF(4250,5500,4250,5500));

    layoutButton = newImageLayout(QString("One 8x10"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(250,500,8000,10000));

    // Select the first layout as default
    OnLayout(imageLayoutList.first());

    // ToDo Make sure we resize the images when the graphicsView is resized.
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(OnResize()));

    // Load the settings
    readSettings();
}

MainWindow::~MainWindow()
{
    if (saveSettingsOnExit) {
        // Make sure any settings are saved
        writeSettings();
    }

    delete signalMapperLayout;
    delete signalMapperPrint;
    delete graphicsScene;
    delete fileSelection;
    delete fileModel;
    delete fileThumbnail;
    delete ui;
    delete settings;
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

void MainWindow::OnDeletePictures()
{
    qDebug() << __FUNCTION__;
}

void MainWindow::LoadImages()
{
    int imageIndex,layoutIndex;
    bool imageLandscape,layoutLandscape;
    double imageAspect,layoutAspect;

    qDebug() << __FUNCTION__ << this->imageLayoutCurr->text();
    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Empty the scene
    graphicsScene->clear();

    // Draw a bounding rectangle for the page
    graphicsScene->addRect(imageLayoutCurr->rect,QPen(QColor(0,0,0)));

    // For Each image location in the layout place an image.
    // Repeat the image if there are more locations than selected images.
    if (indexList.length()) for (imageIndex = 0, layoutIndex = 0; layoutIndex < imageLayoutCurr->getImageCnt(); imageIndex++, layoutIndex++) {

        // Check if we are trying to display more images than selected
        // and loop back to the first image.
        if (imageIndex >= indexList.length()) imageIndex = 0;

        // Load the image into a pixmap
        QPixmap pixmap(fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath());
        imageAspect = pixmap.width() / pixmap.height();
//        qDebug() << fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath() << pixmap.width() << "x" << pixmap.height();

        // Figure out image orientation
        if (imageAspect >= 1.0) {
            // Image is landscape
            imageLandscape = true;
        } else {
            // Image is portrait
            imageLandscape = false;
        }

        // Get the layout rectangle where we want the image
        QRectF rect = imageLayoutCurr->getImageRect(layoutIndex);
        layoutAspect = rect.width() / rect.height();

        // Draw a bounding rectangle
//        graphicsScene->addRect(rect,,QPen(QColor(255,255,255)));

        // Figure out layout position orientation
        if (layoutAspect >= 1.0) {
            // Image Layout position is landscape
            layoutLandscape = true;
        } else {
            // Image Layout position is portrait
            layoutLandscape = false;
        }

        // Put the pixmap on the display
        QGraphicsPixmapItem *item = graphicsScene->addPixmap(pixmap);

        // Local variables
        qreal x,y,r1,r2;

        // Move/Scale the image to the appropriate location
        if (imageLandscape != layoutLandscape) {
            // Different orientations so we need to rotate the image to fit the layout
            item->setRotation(90.0);

            x  = rect.right();
            y  = rect.top();
            r1 = (qreal) rect.width()/(qreal) pixmap.height();
            r2 = (qreal) rect.height()/(qreal) pixmap.width();

            if (r1 > r2) {
                r1 = r2;
            }

            x -= (rect.width() - pixmap.height() * r1) / 2.0;
            y -= (rect.height() - pixmap.width() * r1) / 2.0;
        } else {
            // Same orientation so no rotation necessary
            item->setRotation(0.0);

            x  = rect.left();
            y  = rect.top();
            r1 = (qreal) rect.width()/(qreal) pixmap.width();
            r2 = (qreal) rect.height()/(qreal) pixmap.height();

            if (r1 > r2) {
                r1 = r2;
            }

            x += (rect.width() - pixmap.width() * r1) / 2.0;
            y -= (rect.height() - pixmap.height() * r1) / 2.0;
        }

        item->setScale(r1);
        item->setPos(x,y);
    }
    OnResize();
}

void MainWindow::OnResize()
{
    // Make sure the correct portion of the graphicsScene is visible in the graphicsView.
    ui->graphicsView->setScene(graphicsScene);
    ui->graphicsView->fitInView(graphicsScene->sceneRect(),Qt::KeepAspectRatio);
    ui->graphicsView->ensureVisible(graphicsScene->sceneRect());
}

void MainWindow::OnSelectionChanged(QItemSelection selected,QItemSelection deselected)
{
//    qDebug() << "selected" << selected;
//    qDebug() << "deselected" << deselected;
    LoadImages();
}

void MainWindow::OnLayout(QWidget *widget)
{
    QImageLayoutButton *imageLayout;

    // Set the current layout
    imageLayoutCurr = (QImageLayoutButton*) widget;

    // enable all layouts except the selected one
    int i;
    for (i = 0; i < imageLayoutList.length(); i++) {
        imageLayout = imageLayoutList.at(i);
        imageLayout->setEnabled(imageLayout != imageLayoutCurr);
    }

    LoadImages();
}

void MainWindow::AddPrinter(QPrinter *printer)
{
    printerList.append(printer);

    QPushButton *button = new QPushButton(printer->printerName());
    QAction *actionPrinterRemove = new QAction(tr("Remove Printer"),this);
    QAction *actionPrinterSettings = new QAction(tr("Printer Settings"),this);
    button->addAction(actionPrinterRemove);
    button->addAction(actionPrinterSettings);
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    printButtonList.append(button);
    ui->verticalLayoutPrintButtons->addWidget(button);

    signalMapperPrint->setMapping(button,printButtonList.length() - 1);
    connect(button, SIGNAL(clicked()), signalMapperPrint, SLOT(map()));

    signalMapperPrinterRemove->setMapping(actionPrinterRemove,printButtonList.length() - 1);
    connect(actionPrinterRemove, SIGNAL(triggered()), signalMapperPrinterRemove, SLOT(map()));

    signalMapperPrinterSettings->setMapping(actionPrinterSettings,printButtonList.length() - 1);
    connect(actionPrinterSettings, SIGNAL(triggered()), signalMapperPrinterSettings, SLOT(map()));
}

void MainWindow::OnAddPrinter()
{
//    qDebug() << __FUNCTION__;
    QPrinter *printer = new QPrinter();
    QPrintDialog printDialog(printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        qDebug() << "adding" << printer->printerName() << printer->resolution();
        AddPrinter(printer);
    }
}

void MainWindow::OnPrinterRemove(int index)
{
    // Set the current button
    QPushButton *button = printButtonList.at(index);
    qDebug() << __FUNCTION__ << button->text();

    // Cleanup and remove the button
    ui->verticalLayoutPrintButtons->removeWidget(button);
    disconnect(button, SIGNAL(clicked()));
    button->hide();
    while(button->actions().length()) {
        QAction *action = button->actions().last();
        disconnect(action, SIGNAL(triggered()));
        button->removeAction(action);
        delete action;
    }
    // ToDo delete the button causing SIGABRT
//    delete button;

    // Just replace the entry in the list with NULL so other printer indexes are still correct
    printButtonList.replace(index, NULL);

    // Also cleanup the printer object
    QPrinter *printer = printerList.at(index);
    delete printer;

    // Just replace the entry in the list with NULL so other printer indexes are still correct
    printerList.replace(index, NULL);
}

void MainWindow::OnPrinterSettings(int index)
{
    // Set the current button
    QPushButton *button = printButtonList.at(index);
    qDebug() << __FUNCTION__ << button->text();

    // ToDo: Dialog only flashed and doesn't let you change anything
    QPrintDialog printDialog(printerList.at(index), this);
    printDialog.exec();
}

void MainWindow::OnPrint(int index)
{
    QPrinter *printer = printerList.at(index);
    printer->setCopyCount(ui->spinBoxCopies->value());

    qDebug() << __FUNCTION__ << printer->printerName();

    // Do the printing here

    if (ui->checkBoxPreview->checkState() == Qt::Checked) {
        // Do a preview if checked
        QPrintPreviewDialog printPreview(printer);
        connect(&printPreview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
        printPreview.exec();
        disconnect(this, SLOT(paintRequested(QPrinter*)));
    } else {
        // Else do a print
        paintRequested(printer);
    }

    // If Reset is checked clear the settings
    if (ui->checkBoxReset->checkState() == Qt::Checked) {
        // Set the copies count back to 1
        ui->spinBoxCopies->setValue(1);

        // Set the layout back to default (first entry)
        OnLayout(imageLayoutList.first());

        // Clear the current selection
        fileSelection->clear();

        // Update the display
        LoadImages();
    }
}

void MainWindow::paintRequested(QPrinter *printer)
{
    QPainter painter(printer);
    graphicsScene->render(&painter);
}

void MainWindow::OnDirLoaded(QString dir)
{
    qDebug() << __FUNCTION__ << dir;
    fileModel->sort(3);
    ui->listView->scrollToBottom();
    if (fileSelection->selectedIndexes().length() == 0) {
        // Currently no Items are selected so select the latest
//        ui->listView->sel
    }
}

/*
 * Automatically connected action driven slots
 */
void MainWindow::on_actionFull_Screen_triggered(bool checked)
{
    //qDebug() << __FUNCTION__;
    if (!checked) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::on_actionSave_Settings_triggered(bool checked)
{
    writeSettings();
}

void MainWindow::on_actionSave_Settings_On_Exit_triggered(bool checked)
{
    saveSettingsOnExit = checked;
}

/*
 * Private functions
 */
QImageLayoutButton *MainWindow::newImageLayout(QString name)
{
    QImageLayoutButton *layoutButton;

    layoutButton = new QImageLayoutButton(name);
    imageLayoutList.append(layoutButton);
    ui->gridLayoutLayoutButtons->addWidget(layoutButton,0,imageLayoutList.length());
    signalMapperLayout->setMapping(layoutButton,layoutButton);
    connect(layoutButton, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    return layoutButton;
}

/*
 * Other events
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    OnResize();
}

/*
 * Application settings
 */
void MainWindow::writeSettings()
{
    int i, numItems;
    int numPrintersSaved = 0;
    QString settingBase;

    settings->setValue("MainWindow/fullscreen", isFullScreen());
    settings->setValue("MainWindow/maximize", isMaximized());
    settings->setValue("MainWindow/size", size());
    settings->setValue("MainWindow/pos", pos());
    settings->setValue("CurrentDir", fileModel->rootPath());
    settings->setValue("ThumbNailSize", ui->listView->iconSize());
    settings->setValue("SaveSettingsOnExit", saveSettingsOnExit);
    settings->setValue("ResetOnPrint", ui->checkBoxReset->isChecked());
    settings->setValue("PrintPreview", ui->checkBoxPreview->isChecked());

    numItems = ui->splitter->sizes().length();
    for (i = 0; i < numItems; i++) {
        settingBase = QString("Split");
        settingBase.append(QString::number(i));
        settings->setValue(settingBase, ui->splitter->sizes().at(i));
    }
    settings->setValue("NumSplits", numItems);

    numItems = printerList.length();
    for (i = 0; i < numItems; i++) {
        QPrinter *printer = printerList.at(i);
        if (!printer) continue; // Skip removed printers

        settingBase = QString("Printer/");
        settingBase.append(QString::number(numPrintersSaved));
//        qDebug() << "Saving" << settingBase << printer->printerName();

        settings->beginGroup(settingBase);
        settings->setValue("OutputFormat",printer->outputFormat());
        settings->setValue("Name",printer->printerName());
        settings->setValue("DocName",printer->docName());
        settings->setValue("Creator",printer->creator());
        settings->setValue("Orientation",printer->orientation());
        settings->setValue("PageSize",printer->pageSize());
        settings->setValue("PaperSize",printer->paperSize());
        settings->setValue("PageOrder",printer->pageOrder());
        settings->setValue("Resolution",printer->resolution());
        settings->setValue("ColorMode",printer->colorMode());
        settings->setValue("CollateCopies",printer->collateCopies());
        settings->setValue("FullPage",printer->fullPage());
        settings->setValue("NumCopies",printer->numCopies());
//        settings->setValue("CopyCount",printer->copyCount());
        settings->setValue("PaperSource",printer->paperSource());
        settings->setValue("Duplex",printer->duplex());
        settings->endGroup();
        numPrintersSaved ++;
    }
    settings->setValue("NumPrinters", numPrintersSaved);

    // And flush the settings writes
    settings->sync();
}

void MainWindow::readSettings()
{
    int i, numItems;
    QString settingBase;

    if (settings->value("MainWindow/fullscreen", true).toBool()) {
        showFullScreen();
        ui->actionFull_Screen->setChecked(true);
    } else if (settings->value("MainWindow/maximize", true).toBool()) {
        showMaximized();
        ui->actionFull_Screen->setChecked(false);
    } else {
        showNormal();
        resize(settings->value("MainWindow/size", QSize(720,480)).toSize());
        move(settings->value("MainWindow/pos", QPoint(100,100)).toPoint());
        ui->actionFull_Screen->setChecked(false);
    }

    // Setup the current directory
    QString dirName = QDir::homePath() + "/" + DEFAULT_DIR;
    qDebug() << "dirName" << dirName;
    dirName = settings->value("CurrentDir", dirName).toString();
    fileModel->setRootPath(dirName);

    ui->listView->setIconSize(settings->value("ThumbNailSize",ui->listView->iconSize()).toSize());

    saveSettingsOnExit = settings->value("SaveSettingsOnExit", false).toBool();
    ui->actionSave_Settings_On_Exit->setChecked(saveSettingsOnExit);

    ui->checkBoxReset->setChecked(settings->value("ResetOnPrint", true).toBool());
    ui->checkBoxPreview->setChecked(settings->value("PrintPreview", false).toBool());
//    ui->splitter->setSizes(settings->value("Splits").);

    numItems = settings->value("NumSplits", 3).toInt();
    QList<int> sizeList;
    for (i = 0; i < numItems; i++) {
        settingBase = QString("Split");
        settingBase.append(QString::number(i));
        sizeList.append(settings->value(settingBase, (i + 1) * (720 / 3)).toInt());
    }
    ui->splitter->setSizes(sizeList);

    numItems = settings->value("NumPrinters", 0).toInt();
    for (i = 0; i < numItems; i++) {
        QPrinter *printer = new QPrinter();
        settingBase = QString("Printer/");
        settingBase.append(QString::number(i));
//        qDebug() << "Loading" << settingBase;

        settings->beginGroup(settingBase);
        printer->setOutputFormat((QPrinter::OutputFormat) settings->value("OutputFormat").toInt());
        printer->setPrinterName(settings->value("Name").toString());
        printer->setDocName(settings->value("DocName").toString());
        printer->setCreator(settings->value("Creator").toString());
        printer->setOrientation((QPrinter::Orientation) settings->value("Orientation").toInt());
        printer->setPageSize((QPrinter::PageSize) settings->value("PageSize").toInt());
        printer->setPaperSize((QPrinter::PaperSize) settings->value("PaperSize").toInt());
        printer->setPageOrder((QPrinter::PageOrder) settings->value("PageOrder").toInt());
        printer->setResolution(settings->value("Resolution").toInt());
        printer->setColorMode((QPrinter::ColorMode) settings->value("ColorMode").toInt());
        printer->setCollateCopies(settings->value("CollateCopies").toBool());
        printer->setFullPage(settings->value("FullPage").toBool());
        printer->setNumCopies(settings->value("NumCopies").toInt());
//        printer->setCopyCount(settings->value("CopyCount").toInt());
        printer->setPaperSource((QPrinter::PaperSource) settings->value("PaperSource").toInt());
        printer->setDuplex((QPrinter::DuplexMode) settings->value("Duplex").toInt());
        settings->endGroup();

        AddPrinter(printer);
    }
}
