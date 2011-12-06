#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QFileSystemModel>

#include "imagelayout.h"

namespace Ui {
    class PreviewWindow;
}

class PreviewWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PreviewWindow(QWidget *parent = 0);
    ~PreviewWindow();

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void OnResize();

public:
    Ui::PreviewWindow           *ui;
    QGraphicsScene              *graphicsScene;
    QList<QImageLayoutButton*>   imageLayoutList;
    QImageLayoutButton          *imageLayoutCurr;

    QImageLayoutButton          *newImageLayout(QString name);
};

#endif // PREVIEWWINDOW_H
