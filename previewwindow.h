#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

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
    void OnDirLoaded(QString dir);

private:
    Ui::PreviewWindow           *ui;
    QGraphicsScene              *graphicsScene;
    bool                         loadImagesDisabled;

    void LoadImages();
};

#endif // PREVIEWWINDOW_H
