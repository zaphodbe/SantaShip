#include "previewwindow.h"
#include "ui_previewwindow.h"

#include <QDebug>

PreviewWindow::PreviewWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreviewWindow),
    loadImagesDisabled(FALSE)
{
    // Start the ui engine
    ui->setupUi(this);

    // Setup the graphics scene to paint the images in
    graphicsScene = new QGraphicsScene(this);

    // Load the latest images
    LoadImages();
}

PreviewWindow::~PreviewWindow()
{
    delete ui;
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
void PreviewWindow::LoadImages()
{
    int imageIndex,layoutIndex;
    bool imageLandscape,layoutLandscape;
    double imageAspect,layoutAspect;

    qDebug() << __FUNCTION__;

    if (loadImagesDisabled) return;

    // Empty the scene
    graphicsScene->clear();

    // Draw a bounding rectangle for the page
    graphicsScene->addRect(QRect(0,0,8000,6000),QPen(QColor(0,0,0)));

#if 0
    // Get the list of images to load
    QModelIndexList indexList = fileSelection->selectedIndexes();

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
#endif

    OnResize();
}

void PreviewWindow::OnResize()
{
    qDebug() << __FUNCTION__;

    // Make sure the correct portion of the graphicsScene is visible in the graphicsView.
    ui->graphicsView->setScene(graphicsScene);
    ui->graphicsView->fitInView(graphicsScene->sceneRect(),Qt::KeepAspectRatio);
    ui->graphicsView->ensureVisible(graphicsScene->sceneRect());
}

void PreviewWindow::OnDirLoaded(QString dir)
{
    qDebug() << __FUNCTION__ << dir;
//    fileModel->sort(3);
//    ui->listView->scrollToBottom();
}

