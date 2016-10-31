#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "thumbnail.h"
#include "dialogcloudsetup.h"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsItem>
#include <QPrintPreviewDialog>
#include <QMessageBox>
#include <QDirModel>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>

#include <qts3.h>

#ifndef DEFAULT_DIR
    // Expected to be a subdirectory under the home directory
    #define DEFAULT_DIR "Pictures"
#endif

// Credentials for S3 access
QByteArray awsKeyId = "AKIAIN572KU2X65CCJRA";
QByteArray awsSecretKey = "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";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fileModel(NULL),
    fileSelection(NULL),
    fileThumbnail(NULL),
    changeEnable(false),
    loadImagesDisabled(false),
    previewWindow(new PreviewWindow(this)),
    adminMode(false),
    adminPassword(""),
    dirName(""),
    deselectInProcess(false)
{
    // Start the timer so we can keep track of execution times.
//    qDebug() << __FILE__ << __FUNCTION__ << "Starting mainwindow timer" << timer1.restart();

    // Initialize so we can access the settings
    settings = new QSettings(QString("SantaShip"),QString("SantaShip"));
//    qDebug() << __FILE__ << __FUNCTION__ << settings->fileName();

    // Start the ui engine
    ui->setupUi(this);

    // Put the build date time and version into the labelVersion
    ui->labelVersion->setText(QString("SantaShip Ver 2.1 ") + QString(__DATE__) + QString(" ") + QString(__TIME__));

    // Setup the right click actions that we will add to the buttons
    actionDeletePictures = new QAction(tr("Delete Selected"),this);
    connect(actionDeletePictures, SIGNAL(triggered()), this, SLOT(OnDeletePictures()));

    actionArchivePictures = new QAction(tr("Archive Selected"),this);
    connect(actionArchivePictures, SIGNAL(triggered()), this, SLOT(OnArchive()));

    signalMapperPrinterRemove = new QSignalMapper(this);
    connect(signalMapperPrinterRemove, SIGNAL(mapped(int)), this, SLOT(OnPrinterRemove(int)));

    signalMapperPrinterSettings = new QSignalMapper(this);
    connect(signalMapperPrinterSettings, SIGNAL(mapped(int)), this, SLOT(OnPrinterSettings(int)));

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
    thumbnailTimer = new QTimer();
    fileThumbnail = new QFileThumbnailProvider();

    // Connect the thumbnail timer to force a reload if not events for that amount of time
    connect(thumbnailTimer, SIGNAL(timeout()), this, SLOT(OnThumbnailTimeout()));

    // Initialize the model of the file system
    fileModel = new QFileSystemModel(this);
    fileModel->setNameFilters(filterList);
    fileModel->setFilter(QDir::Files);
    fileModel->setNameFilterDisables(false);
    fileModel->sort(3);

    // Start sending directory updates to the various windows
    connect(fileModel, SIGNAL(directoryLoaded(QString)), this, SLOT(OnDirLoaded(QString)));
//    connect(fileModel, SIGNAL(layoutAboutToBeChanged()), this, SLOT(genIconsStart()));
//    connect(fileModel, SIGNAL(layoutChanged()), this, SLOT(genIconsDone()));
//    connect(fileModel, SIGNAL(directoryLoaded(QString)), previewWindow, SLOT(OnDirLoaded(QString)));

    // Initialize the list view to display the file system model
//    qDebug() << __FILE__ << __FUNCTION__ << "Setup List View:" << timer1.restart();
    ui->listView->setAcceptDrops(false);
    ui->listView->setDragEnabled(false);
    ui->listView->setDragDropMode(QAbstractItemView::NoDragDrop);
    ui->listView->addAction(actionDeletePictures);
    ui->listView->addAction(actionArchivePictures);
    ui->listView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->listView->setIconSize(QSize(250,150));

    // Setup the class to track selections on the list view
    fileSelection = new QItemSelectionModel(fileModel);

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

    layoutButton = newImageLayout(QString("1 Up L"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,11000,8500));

    layoutButton = newImageLayout(QString("2 Up L"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,5500,8500));
    layoutButton->addImage(QRectF(5500,0,5500,8500));

    layoutButton = newImageLayout(QString("4 Up L"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,5500,4250));
    layoutButton->addImage(QRectF(0,4250,5500,4250));
    layoutButton->addImage(QRectF(5500,0,5500,4250));
    layoutButton->addImage(QRectF(5500,4250,5500,4250));

    layoutButton = newImageLayout(QString("One 8x10 L"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(500,250,10000,8000));

    layoutButton = newImageLayout(QString("1 Up P"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,11000));

    layoutButton = newImageLayout(QString("2 Up P"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,5500));
    layoutButton->addImage(QRectF(0,5500,8500,5500));

    layoutButton = newImageLayout(QString("4 Up P"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,4250,5500));
    layoutButton->addImage(QRectF(4250,0,4250,5500));
    layoutButton->addImage(QRectF(0,5500,4250,5500));
    layoutButton->addImage(QRectF(4250,5500,4250,5500));

    layoutButton = newImageLayout(QString("One 8x10 P"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(250,500,8000,10000));

    // Select the first layout as default
    OnLayout(imageLayoutList.first());

    // ToDo Make sure we resize the images when the graphicsView is resized.
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(OnResize()));

    // Load the settings
//    qDebug() << __FILE__ << __FUNCTION__ << "readSettings:" << timer1.restart();
    readSettings();

    // Start the thread to sync with the cloud
    cloudSyncThread.start();

}

MainWindow::~MainWindow()
{
    delete previewWindow;
    delete signalMapperLayout;
    delete signalMapperPrint;
    delete graphicsScene;
    delete fileSelection;
    delete fileModel;
    delete fileThumbnail;
    delete ui;
    delete thumbnailTimer;
    delete settings;
}

/*
 * Normal Event driven slots.
 */

void MainWindow::OnDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,tr("Open Directory"),fileModel->rootPath());

//    qDebug() << __FILE__ << __FUNCTION__ << directory;
    fileModel->setRootPath(directory);
}

void MainWindow::OnSmaller()
{
    QSize size = ui->listView->iconSize();

    size.setWidth(size.width() / 2);
    size.setHeight(size.width() * 3 / 5);

    ui->listView->setIconSize(size);

    ui->pushButtonBigger->setEnabled(isAdminMode());
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

    ui->pushButtonSmaller->setEnabled(isAdminMode());
    if (size.width() >= 500) {
        ui->pushButtonBigger->setEnabled(false);
    }
}

void MainWindow::OnThumbnailTimeout()
{
//    qDebug() << __FILE__ << __FUNCTION__ << "Thumbnails timedout";

    // Toggle the existance of a .touch directory to force folder refresh
    QString touchDir = fileModel->rootPath() + "/.touch";
    QDir dir(touchDir);
    if (!dir.exists())
    {
        // Touch folder doesn't exist so create it
        dir.mkpath(touchDir);
    }
    else
    {
        // Touch folder exist so remove it
        dir.rmdir(touchDir);
    }

}

void MainWindow::restartThumbnailTimer()
{
    // Set timeout to notify app to reload thus refresh thumbnails
    thumbnailTimer->setSingleShot(true);
    thumbnailTimer->start(5000);
}

void MainWindow::OnDeletePictures()
{
    int imageIndex;

    // Disable LoadImages
    loadImagesDisabled = true;

    // Get list of selected files
    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Display warning message box
    QMessageBox msgBox(QMessageBox::Question,"Delete Selected Files", "Are you sure?", QMessageBox::Ok | QMessageBox::Cancel, this);
    int result = msgBox.exec();
    if (result == QMessageBox::Ok) {
        //qDebug() << __FILE__ << __FUNCTION__ << "Delete files";

        // remove the files
        for (imageIndex = 0; imageIndex < indexList.length(); imageIndex++) {
            fileModel->remove(indexList.at(imageIndex));
        }
    }
    // Clear the current selection
    fileSelection->clear();

    // Re-enable LoadImages
    loadImagesDisabled = false;

    // Update the display
    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
}

void MainWindow::LoadImages(QGraphicsScene* graphicsScene, QModelIndexList indexList, QImageLayoutButton* imageLayoutCurr)
{
    int     imageIndex,layoutIndex,firstImageIndex,imageFlags;
    bool    imageLandscape,layoutLandscape;
    double  imageAspect,layoutAspect;

//    qDebug() << __FILE__ << __FUNCTION__ << this->imageLayoutCurr->text();

    if (loadImagesDisabled) return;

//    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Empty the scene
    graphicsScene->clear();

    // Draw a bounding rectangle for the page
    graphicsScene->addRect(imageLayoutCurr->rect,QPen(QColor(0,0,0)));

    // Figure out the first image to use and justify to latest selections
    firstImageIndex = indexList.count() - imageLayoutCurr->getImageCnt();
    if (firstImageIndex < 0) firstImageIndex = 0;

    // For Each image location in the layout place an image.
    // Repeat the image if there are more locations than selected images.
    if (indexList.length()) for (imageIndex = firstImageIndex, layoutIndex = 0; layoutIndex < imageLayoutCurr->getImageCnt(); imageIndex++, layoutIndex++) {

        // Check if we are trying to display more images than selected
        // and loop back to the first image.
        if (imageIndex >= indexList.length()) imageIndex = firstImageIndex;

        // Get the layout rectangle and flags of where we want the image
        QRectF rect = imageLayoutCurr->getImageRect(layoutIndex);
        imageFlags = imageLayoutCurr->getImageFlags(layoutIndex);
        layoutAspect = rect.width() / rect.height();

        // Figure out layout position orientation
        if (layoutAspect >= 1.0) {
            // Image Layout position is landscape
            layoutLandscape = true;
        } else {
            // Image Layout position is portrait
            layoutLandscape = false;
        }

        // Load the image into a pixmap
        QPixmap pixmap(fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath());
        if (pixmap.isNull()) continue;

        imageAspect = pixmap.width() / pixmap.height();

        // Figure out image orientation
        if (imageAspect >= 1.0) {
            // Image is landscape
            imageLandscape = true;
        } else {
            // Image is portrait
            imageLandscape = false;
        }

        // Crop if needed
        if ((imageFlags && QImageLayoutButton::CROP_IMAGE)) {
            int origWidth, origHeight;
            origWidth = pixmap.width();
            origHeight = pixmap.height();

            qDebug() << __FILE__ << __FUNCTION__ << "Cropping from" << origWidth << "x" << origHeight;
        }

        // Local variables
        qreal x,y;
        qreal r1,r2;
        qreal rotation = 0.0;
        qreal newX1 = 0.0;
        qreal newY1 = 0.0;
        qreal newX2 = pixmap.width();
        qreal newY2 = pixmap.height();

        // Move/Scale the image to the appropriate location
        if (imageLandscape != layoutLandscape) {
            // Different orientations so we need to rotate the image to fit the layout
            rotation = 90.0;

            x  = rect.right();
            y  = rect.top();
            r1 = (qreal) rect.width()/(qreal) pixmap.height();
            r2 = (qreal) rect.height()/(qreal) pixmap.width();

            if ((imageFlags && QImageLayoutButton::CROP_IMAGE)) {
                if (r1 < r2) {
                    r1 = r2;
                }
            } else {
                if (r1 > r2) {
                    r1 = r2;
                }
            }

            x -= (rect.width() - pixmap.height() * r1) / 2.0;
            y -= (rect.height() - pixmap.width() * r1) / 2.0;
        } else {
            // Same orientation so no rotation necessary
            rotation = 0.0;

            x  = rect.left();
            y  = rect.top();
            r1 = (qreal) rect.width()/(qreal) pixmap.width();
            r2 = (qreal) rect.height()/(qreal) pixmap.height();

            if ((imageFlags && QImageLayoutButton::CROP_IMAGE)) {
                if (r1 < r2) {
                    r1 = r2;
                }
            } else {
                if (r1 > r2) {
                    r1 = r2;
                }
            }

            x += (rect.width() - pixmap.width() * r1) / 2.0;
            y -= (rect.height() - pixmap.height() * r1) / 2.0;
        }

        // Put the pixmap on the display
        QGraphicsPixmapItem *item = graphicsScene->addPixmap(pixmap.copy(newX1, newY1, newX2, newY2));
        item->setRotation(rotation);
        item->setScale(r1);
        item->setPos(x,y);

        // Draw a bounding rectangle
        graphicsScene->addRect(rect,QPen(QColor(0,0,0)));
    }
}

void MainWindow::OnResize()
{
//    qDebug() << __FILE__ << __FUNCTION__;

    // Make sure the correct portion of the graphicsScene is visible in the graphicsView.
    ui->graphicsView->setScene(graphicsScene);
    ui->graphicsView->fitInView(graphicsScene->sceneRect(),Qt::KeepAspectRatio);
    ui->graphicsView->ensureVisible(graphicsScene->sceneRect());
}

void MainWindow::OnSelectionChanged(QItemSelection selected,QItemSelection deselected)
{
    Q_UNUSED (selected);
    Q_UNUSED (deselected);
    //    qDebug() << __FILE__ << __FUNCTION__ << "selected" << selected.indexes();
    //    qDebug() << __FILE__ << __FUNCTION__ << "deselected" << deselected.indexes();

    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
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

    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
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

void MainWindow::OnPrinterRemove(int index)
{
    // Only do it if we are in admin mode
    if (!isAdminMode()) return;

    // Set the current button
    QPushButton *button = printButtonList.at(index);
//    qDebug() << __FILE__ << __FUNCTION__ << button->text();

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
//    QPushButton *button = printButtonList.at(index);
//    qDebug() << __FILE__ << __FUNCTION__ << button->text();

    // ToDo: Dialog only flashed and doesn't let you change anything
    QPrintDialog printDialog(printerList.at(index), this);
    printDialog.exec();
}

void MainWindow::OnArchive()
{
    int imageIndex;

    // Check for and if necessary create an archive directory
    QString archiveDir = fileModel->rootPath() + "/.Archive";
    QDir dir(archiveDir);
    if (!dir.exists())
    {
        // Archive folder doesn't exist so create it
        dir.mkpath(archiveDir);
    }

    // Now point to the current folder
    dir.setPath(fileModel->rootPath());

    // Disable LoadImages
    loadImagesDisabled = true;

    // Get list of selected files
    QModelIndexList indexList = fileSelection->selectedIndexes();

    // Display warning message box
    QMessageBox msgBox(QMessageBox::Question,"Archive All/Selected Files", "Are you sure?", QMessageBox::Ok | QMessageBox::Cancel, this);
    int result = msgBox.exec();
    if (result == QMessageBox::Ok) {
        //qDebug() << __FILE__ << __FUNCTION__ << "Archive files";
        QString fileName;

        if (indexList.length() == 0) {
            // Nothing is selected move all of them
            QModelIndex index = fileModel->index(dirName);
            int numRows = fileModel->rowCount(index);
            for (int row = 0; row < numRows; row++) {
                fileName = fileModel->fileName(fileModel->index(row,0,index));
                if (!dir.rename(fileName, ".Archive/" + fileName)) {
                    qDebug() << "dir.rename failed!";
                }
            }
        } else {
            // Move selected files
            for (imageIndex = 0; imageIndex < indexList.length(); imageIndex++) {
                fileName = fileModel->fileName(indexList.at(imageIndex));
                if (!dir.rename(fileName, ".Archive/" + fileName)) {
                    qDebug() << "dir.rename failed!";
                }
            }

        }
    }
    // Clear the current selection
    fileSelection->clear();

    // Re-enable LoadImages
    loadImagesDisabled = false;

    // Update the display
    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
}

void MainWindow::OnEMail()
{
    int imageIndex;

    // Get list of selected files
    QModelIndexList indexList = fileSelection->selectedIndexes();

#if 1
    if (ui->lineEditEmail->text() != QString("E-Mail Address") &&
            fileSelection->hasSelection()) {
        // Only do E-Mail/Web if address entered and pictures are selected
        qDebug() << __FILE__ << __FUNCTION__ << "Send E-Mail to " << ui->lineEditEmail->text();

        QString emailID = QUuid::createUuid().toString().mid(1);
        emailID.chop(1);
        QFile emailFile(cloudSyncThread.emailDirName + "/" + emailID + ".eml");
        emailFile.open(QIODevice::Append);
        QTextStream emailStream (&emailFile);

        emailStream << ui->lineEditEmail->text() << "\n";

        qDebug() << "emailID" << emailID;

        // Add the tags for EMail
        // ToDo: Have this stage the EMails
        for (imageIndex = 0; imageIndex < indexList.length(); imageIndex++) {
            QString srcFileName = fileModel->fileName(indexList.at(imageIndex));
            QFileInfo srcFileInfo(srcFileName);
            QString destFileName = QUuid::createUuid().toString().mid(1);
            destFileName.chop(1);
            destFileName.append(".");
            destFileName.append(srcFileInfo.suffix());

            qDebug() << "Copy file" << srcFileName << "to" << destFileName;

            QFile::copy(fileModel->rootPath() + "/" + srcFileName, cloudSyncThread.filesDirName + "/" + destFileName);
            emailStream << destFileName << "\n";
        }

        emailFile.close();
    }
#else
    if (ui->lineEditEmail->text().size() &&
            fileSelection->hasSelection()) {
        // Only do E-Mail if address entered and pictures are selected
        qDebug() << __FILE__ << __FUNCTION__ << "Send E-Mail to " << ui->lineEditEmail->text();

        // Add the tags for EMail
        // ToDo: Have this stage the EMails
        for (imageIndex = 0; imageIndex < indexList.length(); imageIndex++) {
            //fileModel->remove(indexList.at(imageIndex));
            QFile EmailFile("EmailList.txt" /*"C:\WIP\EmailList.txt"*/);
            qDebug() << __FILE__ << __FUNCTION__ << " " << QDateTime::currentDateTime() << ": Send " << fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath() << " to " << ui->lineEditEmail->text();
            if (EmailFile.open(QIODevice::Append)) {
                QTextStream out (&EmailFile);
                out << QDateTime::currentDateTime().toString() << ": Send \"" << fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath() << "\" to \"" << ui->lineEditEmail->text() << "\"\n";
                EmailFile.close();
            } else {
                qDebug() << __FILE__ << __FUNCTION__ << "Failed to open Email List file";
            }
#if 0
            QString url("mailto:");
            url.append(ui->lineEditEmail->text());
            url.append("?subject=File from Santa.&X-Mailer=SantaShip&body=Test message\n\nFrom Santa");
            url.append("&attach=\"");
            url.append(fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath());
            url.append("\"");
            QDesktopServices::openUrl(QUrl(url));
#endif //0
        }
    }
#endif

    // If Reset is checked clear the settings
    if (ui->checkBoxReset->checkState() == Qt::Checked) {
        OnDefaults();
    }
}

void MainWindow::OnPrint(int index)
{
    QPrinter *printer = printerList.at(index);
    printer->setCopyCount(ui->spinBoxCopies->value());

    if (fileSelection->hasSelection()) {
//        qDebug() << __FILE__ << __FUNCTION__ << printer->printerName();

        // Do the printing here

        if (ui->checkBoxPreview->checkState() == Qt::Checked) {
            // Do a preview if checked
            QPrintPreviewDialog printPreview(printer);
            connect(&printPreview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequested(QPrinter*)));
            printPreview.exec();
            disconnect(this, SLOT(paintRequested(QPrinter*)));
        } else {
            // Else do a print
            QPrintDialog printDialog(printer, this);
            if (printDialog.exec() == QDialog::Accepted) {
                paintRequested(printer);
            }
        }
    }

    OnEMail();

    // As we called OnEMail we don't need to do this...
/*
*    // If Reset is checked clear the settings
*    if (ui->checkBoxReset->checkState() == Qt::Checked) {
*        OnDefaults();
*    }
*/
}

void MainWindow::paintRequested(QPrinter *printer)
{
    QPainter painter(printer);
    graphicsScene->render(&painter);
}

void MainWindow::OnDirLoaded(QString dir)
{
//    qDebug() << __FILE__ << __FUNCTION__ << dir << dirName << timer1.restart();

    if (dir == dirName)
    {
        fileModel->setIconProvider(fileThumbnail);
        ui->listView->setModel(fileModel);
        ui->listView->setRootIndex(fileModel->index(dirName));
        ui->listView->setSelectionModel(fileSelection);
    }

#if 0
//    fileModel->sort(3);:"
    ui->listView->scrollToBottom();
    if (fileSelection->selectedIndexes().length() == 0) {
        // Currently no Items are selected so select the latest
//        qDebug() << __FILE__ << __FUNCTION__ << "No files selected so autoselect the last one?";
//        ui->listView->sel
    }
#endif

//    qDebug() << __FILE__ << __FUNCTION__ << "Models Loaded" << timer1.restart();
    loadPreviewWindowContents(dir);
//    qDebug() << __FILE__ << __FUNCTION__ << "Preview Loaded" << timer1.restart();
}

void MainWindow::loadPreviewWindowContents(QString dir)
{
//    qDebug() << __FILE__ << __FUNCTION__ << dir;
    if (previewWindow && previewWindow->isVisible()) {
        QModelIndex index = fileModel->index(dir);
        int numRows = fileModel->rowCount(index);

        // Figure out the start of the last n that will be displayed
        int startRow = numRows - previewWindow->imageLayoutCurr->getImageCnt();
        if (startRow < 0) startRow = 0;

        // Create a list of them so we can use the LoadImages method
        QModelIndexList indexList;
        for (int row = startRow; row < numRows; row++) {
            indexList.append(fileModel->index(row,0,index));
        }

        // Load the images onto the preview window.
        LoadImages(previewWindow->graphicsScene, indexList, previewWindow->imageLayoutCurr);
        previewWindow->OnResize();
    }
}

void MainWindow::OnDefaults()
{
    // Set the E-Mail back
    ui->lineEditEmail->clear();

    // Set the copies count back to 1
    ui->spinBoxCopies->setValue(1);

    // Set the layout back to default (first entry)
    OnLayout(imageLayoutList.first());

    // Clear the current selection
    fileSelection->clear();

    // Update the display
    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
}

/*
 * Automatically connected action driven slots
 */
void MainWindow::on_actionPreview_Window_triggered(bool checked)
{
//    qDebug() << __FILE__ << __FUNCTION__;
    if (checked) {
        // Show the window
        previewWindow->show();
    } else {
        // Hide the window don't destroy it we might want it again
        previewWindow->hide();
    }
}

void MainWindow::on_actionAdd_Printer_triggered(bool checked)
{
    Q_UNUSED(checked);
//    qDebug() << __FILE__ << __FUNCTION__;
    QPrinter *printer = new QPrinter();

    QPrintDialog printDialog(printer, this);
//    qDebug() << __FILE__ << __FUNCTION__ << "First call";
    if (printDialog.exec() == QDialog::Accepted) {
//        qDebug() << __FILE__ << __FUNCTION__ << "adding" << printer->printerName() << printer->resolution();
        AddPrinter(printer);
    }
//    qDebug() << __FILE__ << __FUNCTION__ << "Second call";
//    if (printDialog.exec() == QDialog::Accepted) {
//        qDebug() << __FILE__ << __FUNCTION__ << "adding" << printer->printerName() << printer->resolution();
//        AddPrinter(printer);
//    }
}

void MainWindow::on_actionFull_Screen_triggered(bool checked)
{
    //qDebug() << __FILE__ << __FUNCTION__;
    if (!checked) {
        showNormal();
        previewWindow->showNormal();
    } else {
        showFullScreen();
        previewWindow->showFullScreen();
    }
}

void MainWindow::on_actionSave_Settings_triggered(bool checked)
{
    Q_UNUSED (checked);
    writeSettings();
}

void MainWindow::on_actionChange_Enable_triggered(bool checked)
{
    Q_UNUSED (checked);
    QAction *actClicked = (QAction*) this->sender();
//    qDebug() << __FILE__ << __FUNCTION__ << "Change Enabled" << checked;

    if (adminMode)
    {
        // We are currently in adminMode so we need to get out.
        adminMode = false;
    }
    else
    {
        // We are not in adminMode so we need to get in.
        bool    ok = false;
        QString passwd = QInputDialog::getText(this, QString("Please enter admin password"), QString("Password"), QLineEdit::Password, QString(), &ok);
        if (ok && passwd == adminPassword)
        {
            // User hit ok and passwd matches
            adminMode = true;
        }
        else
        {
            adminMode = false;
        }
    }
    actClicked->setChecked(adminMode);

    // Depending on adminMode enable / disable buttons and menu items.
    ui->actionAdd_Printer->setEnabled(adminMode);
    ui->actionFull_Screen->setEnabled(adminMode);
    ui->actionPreview_Window->setEnabled(adminMode);
    ui->actionSave_Settings->setEnabled(adminMode);
    ui->pushButtonDir->setEnabled(adminMode);
    ui->actionCloud_Access->setEnabled(adminMode);

    // Also enable / disable the size change buttons.
    QSize size = ui->listView->iconSize();
    if (size.width() > 125 )
    {
        ui->pushButtonSmaller->setEnabled(adminMode);
    }
    else
    {
        ui->pushButtonSmaller->setEnabled(false);
    }
    if (size.width() < 500)
    {
        ui->pushButtonBigger->setEnabled(adminMode);
    }
    else
    {
        ui->pushButtonBigger->setEnabled(false);
    }

}

void MainWindow::on_actionCloud_Access_triggered(bool checked)
{
#if 1
    Q_UNUSED (checked);
    DialogCloudSetup cloudSetup;
    cloudSetup.setS3Access(cloudSyncThread.S3Access);
    cloudSetup.setS3Secret(cloudSyncThread.S3Secret);
    cloudSetup.setS3Bucket(cloudSyncThread.S3Bucket);
    cloudSetup.setEMailFrom(cloudSyncThread.emailFrom);
    cloudSetup.setEMailDomain(cloudSyncThread.emailDomain);
    cloudSetup.setEMailUser(cloudSyncThread.emailUser);
    cloudSetup.setEMailPassword(cloudSyncThread.emailPassword);
    cloudSetup.setEMailServer(cloudSyncThread.emailServer);
    cloudSetup.setEMailPort(cloudSyncThread.emailPort);

    int result = cloudSetup.exec();

    if (result == QDialog::Accepted) {
        cloudSyncThread.S3Access = cloudSetup.getS3Access();
        cloudSyncThread.S3Secret = cloudSetup.getS3Secret();
        cloudSyncThread.S3Bucket = cloudSetup.getS3Bucket();
        cloudSyncThread.emailFrom = cloudSetup.getEMailFrom();
        cloudSyncThread.emailDomain = cloudSetup.getEMailDomain();
        cloudSyncThread.emailUser = cloudSetup.getEMailUser();
        cloudSyncThread.emailPassword = cloudSetup.getEMailPassword();
        cloudSyncThread.emailServer = cloudSetup.getEMailServer();
        cloudSyncThread.emailPort = cloudSetup.getEMailPort();
    }
#else
    Q_UNUSED (checked);
    bool    ok = false;
    QString S3Access = QInputDialog::getText(this, QString("Please enter S3 Access ID"), QString("Access ID"), QLineEdit::Normal, cloudSyncThread.S3Access, &ok);
    if (ok)
    {
        // User hit ok
        cloudSyncThread.S3Access = S3Access;
    }
#endif
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
    settings->setValue("ResetOnPrint", ui->checkBoxReset->isChecked());
    settings->setValue("PrintPreview", ui->checkBoxPreview->isChecked());
    settings->setValue("AdminPassword", adminPassword);

    settings->setValue("PreviewWindow/enabled", previewWindow->isVisible());
    settings->setValue("PreviewWindow/fullscreen", previewWindow->isFullScreen());
    settings->setValue("PreviewWindow/maximize", previewWindow->isMaximized());
    settings->setValue("PreviewWindow/size", previewWindow->size());
    settings->setValue("PreviewWindow/pos", previewWindow->pos());

    settings->setValue("S3Access", cloudSyncThread.S3Access);
    settings->setValue("S3Secret", cloudSyncThread.S3Secret);
    settings->setValue("S3Bucket", cloudSyncThread.S3Bucket);

    settings->setValue("EMailUser", cloudSyncThread.emailUser);
    settings->setValue("EMailFrom", cloudSyncThread.emailFrom);
    settings->setValue("EMailDomain", cloudSyncThread.emailDomain);
    settings->setValue("EMailPassword", cloudSyncThread.emailPassword);
    settings->setValue("EMailServer", cloudSyncThread.emailServer);
    settings->setValue("EMailPort", cloudSyncThread.emailPort);

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
//        qDebug() << __FILE__ << __FUNCTION__ << "Saving" << settingBase << printer->printerName();

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

    // Get the admin password if set
    adminPassword = settings->value("AdminPassword", QString("")).toString();

    // Load MainWindow settings
    resize(settings->value("MainWindow/size", QSize(720,480)).toSize());
    move(settings->value("MainWindow/pos", QPoint(100,100)).toPoint());
    if (settings->value("MainWindow/fullscreen", true).toBool()) {
        showFullScreen();
        ui->actionFull_Screen->setChecked(true);
    } else if (settings->value("MainWindow/maximize", true).toBool()) {
        showMaximized();
        ui->actionFull_Screen->setChecked(false);
    } else {
        showNormal();
        ui->actionFull_Screen->setChecked(false);
    }

    // Load PreviewWindow settings
    previewWindow->setVisible(settings->value("PreviewWindow/enabled", false).toBool());
    if (previewWindow->isVisible()) {
        previewWindow->resize(settings->value("PreviewWindow/size", QSize(720,480)).toSize());
        previewWindow->move(settings->value("PreviewWindow/pos", QPoint(300,300)).toPoint());
        if (settings->value("PreviewWindow/fullscreen", true).toBool()) {
            previewWindow->showFullScreen();
        } else if (settings->value("PreviewWindow/maximize", true).toBool()) {
            previewWindow->showMaximized();
        } else {
            previewWindow->showNormal();
        }
    }
    ui->actionPreview_Window->setChecked(previewWindow->isVisible());

    // Setup the current directory
    dirName = QDir::homePath() + "/" + DEFAULT_DIR;
    dirName = settings->value("CurrentDir", dirName).toString();
//    qDebug() << __FILE__ << __FUNCTION__ << "dirName" << dirName;
    fileModel->setRootPath(dirName);
    cloudSyncThread.filesDirName = dirName + "/.AWS";
    cloudSyncThread.emailDirName = dirName + "/.EMail";

    ui->listView->setIconSize(settings->value("ThumbNailSize",ui->listView->iconSize()).toSize());
//    ui->listView->setRootIndex(fileModel->index(dirName));

    ui->checkBoxReset->setChecked(settings->value("ResetOnPrint", true).toBool());
    ui->checkBoxPreview->setChecked(settings->value("PrintPreview", false).toBool());
//    ui->splitter->setSizes(settings->value("Splits").);

    cloudSyncThread.S3Access = settings->value("S3Access").toString();
    cloudSyncThread.S3Secret = settings->value("S3Secret").toString();
    cloudSyncThread.S3Bucket = settings->value("S3Bucket").toString();

    cloudSyncThread.emailUser = settings->value("EMailUser").toString();
    cloudSyncThread.emailFrom = settings->value("EMailFrom").toString();
    cloudSyncThread.emailDomain = settings->value("EMailDomain").toString();
    cloudSyncThread.emailPassword = settings->value("EMailPassword").toString();
    cloudSyncThread.emailServer = settings->value("EMailServer").toString();
    cloudSyncThread.emailPort = settings->value("EMailPort").toInt();

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
//        qDebug() << __FILE__ << __FUNCTION__ << "Loading" << settingBase;

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

bool MainWindow::isAdminMode()
{
    return adminMode;
}

void MainWindow::genIconsStart()
{
    ui->listView->setUpdatesEnabled(false);
}

void MainWindow::genIconsDone()
{
    ui->listView->setUpdatesEnabled(true);
}
