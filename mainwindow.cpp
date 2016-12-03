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
#include <QBitmap>

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
    loadLayouts();

    // Setup the overlay location and scale combo's
    ui->comboBoxOverlayLocation->addItem("Bottom Right",OVERLAY_BOTTOMRIGHT);
    ui->comboBoxOverlayLocation->addItem("Bottom Left",OVERLAY_BOTTOMLEFT);
    ui->comboBoxOverlayLocation->addItem("Top Left",OVERLAY_TOPLEFT);
    ui->comboBoxOverlayLocation->addItem("Top Right",OVERLAY_TOPRIGHT);
    ui->comboBoxOverlayLocation->addItem("Whole Image",OVERLAY_WHOLEIMAGE);

    // ToDo Make sure we resize the images when the graphicsView is resized.
    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(OnResize()));

    // Load the settings
//    qDebug() << __FILE__ << __FUNCTION__ << "readSettings:" << timer1.restart();
    readSettings();

    // Select the first layout as default
    OnLayout(imageLayoutList.first());

    // Start the sync with the cloud
    //cloudSync.start();
    cloudSyncTimer = new QTimer();
    connect(cloudSyncTimer, SIGNAL(timeout()), this, SLOT(OnCloudSyncTimeout()));
    cloudSyncTimer->start(10000);

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
    delete cloudSyncTimer;
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
    OnOverlay();
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

void MainWindow::OnCloudSyncTimeout()
{
    // Use the global thread pool
    if (!cloudSync.data.working) {
        cloudSync.data.working = true;
        QFuture<void> result = QtConcurrent::run(cloudSyncWork, &cloudSync.data);
    }
}

void MainWindow::OnCrop()
{
    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
}

void MainWindow::OnOverlay(QString text)
{
    Q_UNUSED(text);
    OnOverlay();
}

void MainWindow::OnOverlay()
{
    if (ui->comboBoxOverlay->currentIndex()) {
        overlayPixmap = new QPixmap(dirName + "/.Overlays/" + ui->comboBoxOverlay->currentText());
        if (overlayPixmap) {
            QBitmap overlayMask = overlayPixmap->createHeuristicMask();
            if (ui->checkBoxOverlayMask->isChecked())
                overlayPixmap->setMask(overlayMask);
        }
    } else {
        if (overlayPixmap) {
            delete overlayPixmap;
            overlayPixmap = NULL;
        }
    }

    LoadImages(graphicsScene, fileSelection->selectedIndexes(), imageLayoutCurr);
    OnResize();
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
        QRectF layoutRect = imageLayoutCurr->getImageRect(layoutIndex);
        imageFlags = imageLayoutCurr->getImageFlags(layoutIndex);
        layoutAspect = layoutRect.width() / layoutRect.height();

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

        // Local variables
        qreal x,y;
        qreal r1,r2;
        qreal rotation = 0.0;

        // Move/Scale the image to the appropriate location
        if (imageLandscape != layoutLandscape) {
            // Different orientations so we need to rotate the image to fit the layout
            rotation = 90;
            QTransform trans;
            trans.rotate(rotation);
            pixmap = pixmap.transformed(trans);
        }

        qreal newX1 = 0.0;
        qreal newY1 = 0.0;
        qreal newX2 = pixmap.width();
        qreal newY2 = pixmap.height();

        x  = layoutRect.left();
        y  = layoutRect.top();
        r1 = (qreal) layoutRect.width()/(qreal) pixmap.width();
        r2 = (qreal) layoutRect.height()/(qreal) pixmap.height();

        if ((imageFlags & QImageLayoutButton::CROP_IMAGE) || ui->checkBoxCrop->isChecked()) {
            if (r1 < r2) {
                r1 = r2;
            }
            newX1 = ((pixmap.width() * r1) - layoutRect.width()) / (2 * r1);
            newY1 = ((pixmap.height() * r1) - layoutRect.height()) / (2 * r1);
            newX2 -= 2 * newX1;
            newY2 -= 2 * newY1;
            x += newX1 * r1;
            y += newY1 * r1;
        } else {
            if (r1 > r2) {
                r1 = r2;
            }
        }

        x += (layoutRect.width() - pixmap.width() * r1) / 2.0;
        y += (layoutRect.height() - pixmap.height() * r1) / 2.0;

        // Put the pixmap on the display
        QGraphicsPixmapItem *item = graphicsScene->addPixmap(pixmap.copy(newX1, newY1, newX2, newY2));
        item->setScale(r1);
        item->setPos(x,y);

        // Draw a bounding rectangle
        graphicsScene->addRect(layoutRect,QPen(QColor(0,0,0)));

        // Add any specified overlay
        if (overlayPixmap) {
            qreal overlayScale = ui->spinBoxOverlayScale->value() / 100.0;
            r1 = (qreal) layoutRect.width()/(qreal) overlayPixmap->width()*overlayScale;
            r2 = (qreal) layoutRect.height()/(qreal) overlayPixmap->height()*overlayScale;
            if (r1 > r2)
                r1 = r2;

            item = graphicsScene->addPixmap(*overlayPixmap);
            item->setScale(r1);

            x  = layoutRect.left();
            y  = layoutRect.top();
            if (overlayScale != 1.0) {
                switch(ui->comboBoxOverlayLocation->currentData().toInt()) {
                case OVERLAY_TOPLEFT:
                    x += layoutRect.width() * overlayScale / 2;
                    y += layoutRect.height() * overlayScale / 2;
                    break;

                case OVERLAY_TOPRIGHT:
                    x += layoutRect.width() - layoutRect.width() * overlayScale / 2 - overlayPixmap->width() * r1;
                    y += layoutRect.height() * overlayScale / 2;
                    break;

                case OVERLAY_BOTTOMLEFT:
                    x += layoutRect.width() * overlayScale / 2;
                    y += layoutRect.height() - layoutRect.height() * overlayScale / 2 - overlayPixmap->height() * r1;
                    break;

                case OVERLAY_BOTTOMRIGHT:
                    x += layoutRect.width() - layoutRect.width() * overlayScale / 2 - overlayPixmap->width() * r1;
                    y += layoutRect.height() - layoutRect.height() * overlayScale / 2 - overlayPixmap->height() * r1;
                    break;

                case OVERLAY_WHOLEIMAGE:
                    // No changes
                    break;
                }
            }
            item->setPos(x,y);
        }
    }

    qreal border = 0;
    graphicsScene->setSceneRect(imageLayoutCurr->rect.left() - border, imageLayoutCurr->rect.top() - border, imageLayoutCurr->rect.width() + 2 * border, imageLayoutCurr->rect.height() + 2 * border);
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

    int index;
    for (index = 0 ; index < printerList.size() ; index++) {
        QPrinter *printer = printerList.at(index);
        qDebug() << "printer" << printer->printerName();
        qDebug() << "  paperSize" << printer->paperSize(QPrinter::Inch);
        qDebug() << "  imageLayout.rect" << imageLayoutCurr->rect;

        QSizeF paperSize = printer->paperSize(QPrinter::Inch);
        if ((paperSize.width() * 1000 == imageLayoutCurr->rect.width() &&
             paperSize.height() * 1000 == imageLayoutCurr->rect.height()) ||
            (paperSize.width() * 1000 == imageLayoutCurr->rect.height() &&
             paperSize.height() * 1000 == imageLayoutCurr->rect.width())) {

            qDebug() << "  enabled";

            printButtonList.at(index)->setEnabled(true);
        } else {
            qDebug() << "  disabled";

            printButtonList.at(index)->setEnabled(false);
        }
    }

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

    // Do an on layout so printer buttons are enabled accordingly
    //OnLayout(imageLayoutCurr);
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

    if (ui->lineEditEmail->text().size() &&
            ui->lineEditEmail->text().contains("@") &&
            fileSelection->hasSelection()) {
        // Only do E-Mail/Web if address entered and pictures are selected
        qDebug() << __FILE__ << __FUNCTION__ << "Send E-Mail to " << ui->lineEditEmail->text();

        // Create a file containing the email address
        QString emailID = QUuid::createUuid().toString().mid(1);
        emailID.chop(1);
        QFile emailFile(cloudSync.data.emailDirName + "/" + emailID + ".eml");
        emailFile.open(QIODevice::Append);
        QTextStream emailStream (&emailFile);

        emailStream << ui->lineEditEmail->text() << "\n";

        qDebug() << "emailID" << emailID;

        // Add the picture ID's and copy the pictures to the staging folder
        for (imageIndex = 0; imageIndex < indexList.length(); imageIndex++) {
            QString srcFileName = fileModel->fileName(indexList.at(imageIndex));
            QFileInfo srcFileInfo(srcFileName);
            QString destFileName = QUuid::createUuid().toString().mid(1);
            destFileName.chop(1);
            destFileName.append(".");
            destFileName.append(srcFileInfo.suffix());

            qDebug() << "Copy file" << srcFileName << "to" << destFileName;

            QFile::copy(fileModel->rootPath() + "/" + srcFileName, cloudSync.data.filesDirName + "/" + destFileName);
            emailStream << destFileName << "\n";
        }

        emailFile.close();
    }

    // If Reset is checked clear the settings
    if (ui->checkBoxReset->checkState() == Qt::Checked) {
        OnDefaults();
    }
}

void MainWindow::OnPrint(int index)
{
    QPrinter *printer = printerList.at(index);
    printer->setCopyCount(ui->spinBoxCopies->value());

    if (printer && fileSelection->hasSelection()) {
//        qDebug() << __FILE__ << __FUNCTION__ << printer->printerName();

        // Do the printing here
        if (graphicsScene->sceneRect().width() > graphicsScene->sceneRect().height()) {
            printer->setOrientation(QPrinter::Landscape);
        } else {
            printer->setOrientation(QPrinter::Portrait);
        }

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
    // Set the layout back to default (first entry)
    OnLayout(imageLayoutList.first());

    // Set the overlay back to defaults
    ui->comboBoxOverlay->setCurrentIndex(1);
    ui->comboBoxOverlayLocation->setCurrentIndex(0);
    ui->spinBoxOverlayScale->setValue(25);
    ui->checkBoxOverlayMask->setChecked(true);
    OnOverlay();

    // Set the E-Mail back
    ui->lineEditEmail->clear();

    // Set Crop
    ui->checkBoxCrop->setChecked(true);

    // Clear the preview option
    ui->checkBoxPreview->setChecked(false);

    // Set the reset on print
    ui->checkBoxReset->setChecked(true);

    // Set the copies count back to 1
    ui->spinBoxCopies->setValue(1);

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
    Q_UNUSED (checked);
    DialogCloudSetup cloudSetup;
    cloudSetup.setS3Access(cloudSync.data.S3Access);
    cloudSetup.setS3Secret(cloudSync.data.S3Secret);
    cloudSetup.setS3Bucket(cloudSync.data.S3Bucket);
    cloudSetup.setEMailFrom(cloudSync.data.emailFrom);
    cloudSetup.setEMailDomain(cloudSync.data.emailDomain);
    cloudSetup.setEMailUser(cloudSync.data.emailUser);
    cloudSetup.setEMailPassword(cloudSync.data.emailPassword);
    cloudSetup.setEMailServer(cloudSync.data.emailServer);
    cloudSetup.setEMailPort(cloudSync.data.emailPort);
    cloudSetup.setEMailTransport(cloudSync.data.emailTransport);

    int result = cloudSetup.exec();

    if (result == QDialog::Accepted) {
        cloudSync.data.S3Access = cloudSetup.getS3Access();
        cloudSync.data.S3Secret = cloudSetup.getS3Secret();
        cloudSync.data.S3Bucket = cloudSetup.getS3Bucket();
        cloudSync.data.emailFrom = cloudSetup.getEMailFrom();
        cloudSync.data.emailDomain = cloudSetup.getEMailDomain();
        cloudSync.data.emailUser = cloudSetup.getEMailUser();
        cloudSync.data.emailPassword = cloudSetup.getEMailPassword();
        cloudSync.data.emailServer = cloudSetup.getEMailServer();
        cloudSync.data.emailPort = cloudSetup.getEMailPort();
        cloudSync.data.emailTransport = cloudSetup.getEMailTransport();
    }
}

/*
 * Private functions
 */
QImageLayoutButton *MainWindow::newImageLayout(QString name, int row)
{
    QImageLayoutButton *layoutButton;

    layoutButton = new QImageLayoutButton(name);
    imageLayoutList.append(layoutButton);
    int col = 0;
    while (ui->gridLayoutLayoutButtons->itemAtPosition(row,col)) col++;
    ui->gridLayoutLayoutButtons->addWidget(layoutButton,row,col);
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

void MainWindow::loadLayouts()
{
    // ToDo load these from discription files
    QImageLayoutButton *layoutButton;

    layoutButton = newImageLayout(QString("1 4x6L on 4x6"));
    layoutButton->setRect(0,0,6000,4000);
    layoutButton->addImage(QRectF(0,0,6000,4000), QImageLayoutButton::CROP_IMAGE);

    layoutButton = newImageLayout(QString("1 8.5x11L on 8.5x11"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,11000,8500));

    layoutButton = newImageLayout(QString("2 5.5x8.5L on 8.5x11"));
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,5500));
    layoutButton->addImage(QRectF(0,5500,8500,5500));

    layoutButton = newImageLayout(QString("4 4.25x5.5L on 8.5x11"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,5500,4250));
    layoutButton->addImage(QRectF(0,4250,5500,4250));
    layoutButton->addImage(QRectF(5500,0,5500,4250));
    layoutButton->addImage(QRectF(5500,4250,5500,4250));

    layoutButton = newImageLayout(QString("1 8x10L on 8.5x11"));
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(500,250,10000,8000));
#if 0
    layoutButton = newImageLayout(QString("1 4x6 P on 4x6"),1);
    layoutButton->setRect(0,0,4000,6000);
    layoutButton->addImage(QRectF(0,0,4000,6000), QImageLayoutButton::CROP_IMAGE);

    layoutButton = newImageLayout(QString("1 8.5x11 P on 8.5x11"),1);
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,8500,11000));

    layoutButton = newImageLayout(QString("2 5.5x8.5 P on 8.5x11"),1);
    layoutButton->setRect(0,0,11000,8500);
    layoutButton->addImage(QRectF(0,0,5500,8500));
    layoutButton->addImage(QRectF(5500,0,5500,8500));

    layoutButton = newImageLayout(QString("4 4.25x5.5 P on 8.5x11"),1);
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(0,0,4250,5500));
    layoutButton->addImage(QRectF(4250,0,4250,5500));
    layoutButton->addImage(QRectF(0,5500,4250,5500));
    layoutButton->addImage(QRectF(4250,5500,4250,5500));

    layoutButton = newImageLayout(QString("1 8x10 P on 8.5x11"),1);
    layoutButton->setRect(0,0,8500,11000);
    layoutButton->addImage(QRectF(250,500,8000,10000));
#endif
    // Select the first layout as default
    OnLayout(imageLayoutList.first());
}

void MainWindow::loadOverlays()
{
    QString overlayDirName = dirName + "/.Overlays";
    QDir overlayDir(overlayDirName);
    if (!overlayDir.exists())
    {
        // Overlay folder doesn't exist so create it
        overlayDir.mkpath(overlayDirName);
    }
    overlayDir.cd(overlayDirName);

    // Clear any existing overlays
    ui->comboBoxOverlay->clear();

    // Add the no overlay to the list
    ui->comboBoxOverlay->addItem("<No Overlay>"); // Should always be index 0

    // Add the possible overlays to the list and select the first as default
    overlayFiles = overlayDir.entryList(QDir::Files);
    if (overlayFiles.size()) {
        ui->comboBoxOverlay->addItems(overlayFiles);
//        ui->comboBoxOverlay->setCurrentIndex(1);
        OnOverlay();
    }
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

    settings->setValue("S3Access", cloudSync.data.S3Access);
    settings->setValue("S3Secret", cloudSync.data.S3Secret);
    settings->setValue("S3Bucket", cloudSync.data.S3Bucket);

    settings->setValue("EMailUser", cloudSync.data.emailUser);
    settings->setValue("EMailFrom", cloudSync.data.emailFrom);
    settings->setValue("EMailDomain", cloudSync.data.emailDomain);
    settings->setValue("EMailPassword", cloudSync.data.emailPassword);
    settings->setValue("EMailServer", cloudSync.data.emailServer);
    settings->setValue("EMailPort", cloudSync.data.emailPort);
    settings->setValue("EMailTransport", cloudSync.data.emailTransport);

    settings->setValue("Overlay/index", ui->comboBoxOverlay->currentIndex());
    settings->setValue("Overlay/loc", ui->comboBoxOverlayLocation->currentIndex());
    settings->setValue("Overlay/scale", ui->spinBoxOverlayScale->value());
    settings->setValue("Overlay/mask", ui->checkBoxOverlayMask->isChecked());

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
    cloudSync.data.filesDirName = dirName + "/.AWS";
    cloudSync.data.emailDirName = dirName + "/.EMail";
    QDir filesDir(cloudSync.data.filesDirName);
    QDir emailDir(cloudSync.data.emailDirName);

    // Check for and if necessary create the AWS upload directory
    if (!filesDir.exists())
    {
        // EMail folder doesn't exist so create it
        filesDir.mkpath(cloudSync.data.filesDirName);
    }

    // Check for and if necessary create the EMail directory
    if (!emailDir.exists())
    {
        // EMail folder doesn't exist so create it
        emailDir.mkpath(cloudSync.data.emailDirName);
    }

    loadOverlays();
    ui->comboBoxOverlay->setCurrentIndex(settings->value("Overlay/index", 0).toInt());
    ui->comboBoxOverlayLocation->setCurrentIndex(settings->value("Overlay/loc",0).toInt());
    ui->spinBoxOverlayScale->setValue(settings->value("Overlay/scale",25).toInt());
    ui->checkBoxOverlayMask->setChecked(settings->value("Overlay/mask",true).toBool());
    OnOverlay();

    ui->listView->setIconSize(settings->value("ThumbNailSize",ui->listView->iconSize()).toSize());
//    ui->listView->setRootIndex(fileModel->index(dirName));

    ui->checkBoxReset->setChecked(settings->value("ResetOnPrint", true).toBool());
    ui->checkBoxPreview->setChecked(settings->value("PrintPreview", false).toBool());
//    ui->splitter->setSizes(settings->value("Splits").);

    cloudSync.data.S3Access = settings->value("S3Access").toString();
    cloudSync.data.S3Secret = settings->value("S3Secret").toString();
    cloudSync.data.S3Bucket = settings->value("S3Bucket").toString();

    cloudSync.data.emailUser = settings->value("EMailUser").toString();
    cloudSync.data.emailFrom = settings->value("EMailFrom").toString();
    cloudSync.data.emailDomain = settings->value("EMailDomain").toString();
    cloudSync.data.emailPassword = settings->value("EMailPassword").toString();
    cloudSync.data.emailServer = settings->value("EMailServer").toString();
    cloudSync.data.emailPort = settings->value("EMailPort").toInt();
    cloudSync.data.emailTransport = settings->value("EMailTransport").toString();

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
