#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPushButton>
#include <QSignalMapper>
#include <QSettings>

#include "filethumbnailprovider.h"
#include "imagelayout.h"
#include "previewwindow.h"
#include "cloudsync.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum {
        OVERLAY_TOPLEFT,
        OVERLAY_TOPRIGHT,
        OVERLAY_BOTTOMRIGHT,
        OVERLAY_BOTTOMLEFT,
        OVERLAY_WHOLEIMAGE,
    };

private:
    explicit MainWindow(QWidget *parent = 0);

public:
    ~MainWindow();

    bool                         isAdminMode();

    static MainWindow           *getInstance()
    {
        static MainWindow       *instance;
        if (!instance)
        {
            instance = new MainWindow();
        }
        return instance;
    }

protected:
    void resizeEvent(QResizeEvent *event);

    void loadLayouts();
    void loadOverlays();

signals:

public slots:
    void OnDir();
    void OnSmaller();
    void OnBigger();
    void OnSelectionChanged(QItemSelection selected,QItemSelection deselected);
    void OnPrinterRemove(int index);
    void OnPrinterSettings(int index);
    void OnPrint(int index);
    void OnLayout(QWidget *widget);
    void OnDeletePictures();
    void OnResize();
    void OnDirLoaded(QString dir);
    void OnDefaults();
    void OnEMail();
    void OnArchive();
    void OnThumbnailTimeout();
    void OnCloudSyncTimeout();
    void OnCrop();
    void OnOverlay(QString text);
    void OnOverlay();

    void genIconsStart();
    void genIconsDone();

    void on_actionAdd_Printer_triggered(bool checked);
    void on_actionFull_Screen_triggered(bool checked);
    void on_actionSave_Settings_triggered(bool checked);
    void on_actionPreview_Window_triggered(bool checked);
    void on_actionChange_Enable_triggered(bool checked);
    void on_actionCloud_Access_triggered(bool checked);

    void paintRequested(QPrinter* printer);
    void writeSettings();
    void readSettings();
    void restartThumbnailTimer();
    void setVersionLabel(int pics, int emails);

private:
    Ui::MainWindow              *ui;
    QFileSystemModel            *fileModel;
    QItemSelectionModel         *fileSelection;
    QFileThumbnailProvider      *fileThumbnail;
    QList<QPrinter*>             printerList;
    QList<QPushButton*>          printButtonList;
    QGraphicsScene              *graphicsScene;
    QSignalMapper               *signalMapperPrint;
    QSignalMapper               *signalMapperLayout;
    QSignalMapper               *signalMapperPrinterSettings;
    QSignalMapper               *signalMapperPrinterRemove;
    QList<QImageLayoutButton*>   imageLayoutList;
    QImageLayoutButton          *imageLayoutCurr;
    QSettings                   *settings;
    QAction                     *actionDeletePictures;
    QAction                     *actionArchivePictures;
    QAction                     *actionPrinterSettings;
    QAction                     *actionPrinterRemove;
    bool                         changeEnable;
    bool                         loadImagesDisabled;
    PreviewWindow               *previewWindow;
    bool                         adminMode;
    QString                      adminPassword;
    QTime                        timer1;
    QString                      dirName;
    bool                         deselectInProcess;
    QTimer                      *thumbnailTimer;
    QTimer                      *cloudSyncTimer;
    cloudSyncThread              cloudSync;
    QStringList                  overlayFiles;
    QPixmap                     *overlayPixmap;

    void                         LoadImages(QGraphicsScene* graphicsScene, QModelIndexList indexList, QImageLayoutButton* imageLayoutCurr);
    QImageLayoutButton          *newImageLayout(QString name, int row = 0);
    void                         AddPrinter(QPrinter *printer);
    void                         loadPreviewWindowContents(QString dir);
};

#endif // MAINWINDOW_H
