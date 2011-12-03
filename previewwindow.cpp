#include "previewwindow.h"
#include "ui_previewwindow.h"

#include <QDebug>
#include <QGraphicsItem>

PreviewWindow::PreviewWindow(QFileSystemModel* fileModel, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreviewWindow),
    fileModel(fileModel)
{
    // Start the ui engine
    ui->setupUi(this);

    // Setup the graphics scene to paint the images in
    graphicsScene = new QGraphicsScene(this);

    // Setup the Layout.
    // ToDo load this from discription files
    QImageLayoutButton *layoutButton;

    layoutButton = newImageLayout(QString("Preview"));
    layoutButton->setRect(0,0,8000,6000);  // Just something 4:3 aspect ratio
    layoutButton->addImage(QRectF(0,0,4000,3000));
    layoutButton->addImage(QRectF(4000,0,4000,3000));
    layoutButton->addImage(QRectF(0,3000,4000,3000));
    layoutButton->addImage(QRectF(4000,3000,4000,3000));
    layoutButton->addImage(QRectF(2000,1500,4000,3000));
    imageLayoutCurr = layoutButton;
}

PreviewWindow::~PreviewWindow()
{
    delete graphicsScene;
    delete ui;
}

/*
 * Private functions
 */
QImageLayoutButton *PreviewWindow::newImageLayout(QString name)
{
    QImageLayoutButton *layoutButton;

    layoutButton = new QImageLayoutButton(name);
    imageLayoutList.append(layoutButton);
//    ui->gridLayoutLayoutButtons->addWidget(layoutButton,0,imageLayoutList.length());
//    signalMapperLayout->setMapping(layoutButton,layoutButton);
//    connect(layoutButton, SIGNAL(clicked()), signalMapperLayout, SLOT(map()));

    return layoutButton;
}

/*
 * Other events
 */
void PreviewWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    OnResize();
}

/*
 * Normal Event driven slots.
 */
void PreviewWindow::OnResize()
{
//    qDebug() << __FILE__ << __FUNCTION__;

    // Make sure the correct portion of the graphicsScene is visible in the graphicsView.
    ui->graphicsView->setScene(graphicsScene);
    ui->graphicsView->fitInView(graphicsScene->sceneRect(),Qt::KeepAspectRatio);
    ui->graphicsView->ensureVisible(graphicsScene->sceneRect());
}

void PreviewWindow::OnDirLoaded(QString dir)
{
    qDebug() << __FILE__ << __FUNCTION__ << dir;
    fileModel->sort(3);

    int imageIndex,layoutIndex;
    bool imageLandscape,layoutLandscape;
    double imageAspect,layoutAspect;

    // Empty the scene
    graphicsScene->clear();

    // Draw a bounding rectangle for the page
    graphicsScene->addRect(imageLayoutCurr->rect,QPen(QColor(0,0,0)));

    // Get the list of images to load
//    QModelIndexList indexList = fileSelection->selectedIndexes();
    QModelIndexList indexList;

    qDebug() << "rows" << fileModel->rowCount();
    qDebug() << "cols" << fileModel->columnCount();

    for (imageIndex = fileModel->rowCount() - imageLayoutCurr->getImageCnt(); imageIndex < fileModel->rowCount(); imageIndex++) {
        indexList.append(fileModel->index(imageIndex, 0));
    }

//    for (layoutIndex = 0; layoutIndex < imageLayoutCurr->getImageCnt(); layoutIndex++) {
//        indexList.append(fileModel->index());
//    }

    // For Each image location in the layout place an image.
    // Repeat the image if there are more locations than selected images.
    if (indexList.length()) for (imageIndex = 0, layoutIndex = 0; layoutIndex < imageLayoutCurr->getImageCnt(); imageIndex++, layoutIndex++) {

        // Check if we are trying to display more images than selected
        // and loop back to the first image.
        if (imageIndex >= indexList.length()) imageIndex = 0;

        // Load the image into a pixmap
        QPixmap pixmap(fileModel->fileInfo(indexList.at(imageIndex)).absoluteFilePath());
        if (pixmap.isNull()) continue;

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

