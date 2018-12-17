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
    explicit PreviewWindow(QWidget *parent = nullptr);
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

    QImageLayoutButton          *newImageLayout(const QString& name);
};

#endif // PREVIEWWINDOW_H
