#include "previewwindow.h"
#include "ui_previewwindow.h"

#include <QDebug>
#include <QGraphicsItem>

PreviewWindow::PreviewWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreviewWindow)
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
