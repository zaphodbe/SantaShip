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
    explicit PreviewWindow(QFileSystemModel* fileModel, QWidget *parent = 0);
    ~PreviewWindow();

    void setFileSystemModel(QFileSystemModel*   fileModel)  {this->fileModel = fileModel;};
    QFileSystemModel* getFileSystemModel()                  {return(this->fileModel);};

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void OnResize();
    void OnDirLoaded(QString dir);

private:
    Ui::PreviewWindow           *ui;
    QFileSystemModel            *fileModel;
    QGraphicsScene              *graphicsScene;
    QList<QImageLayoutButton*>   imageLayoutList;
    QImageLayoutButton          *imageLayoutCurr;

    QImageLayoutButton          *newImageLayout(QString name);
};

#endif // PREVIEWWINDOW_H
