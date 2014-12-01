#include "filethumbnailprovider.h"

#include <QDebug>
#include <QDir>
#include <QTime>
#include <QStringList>
#include <QtCore>
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QStaticText>

#include "thumbnail.h"

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

#ifndef USE_SCALED_ICON_HEIGHT
#define USE_SCALED_ICON_HEIGHT 150
#endif

QStringList      processingThumbnails;

/* This is the re-implementation to handle thumbnails in the icons */
QFileThumbnailProvider::QFileThumbnailProvider()
{
    // Create the temporary pixmap at runtime so it is the correct size
    pixmapTemp = new QPixmap( USE_SCALED_ICON_WIDTH, USE_SCALED_ICON_HEIGHT);
    pixmapTemp->load(":/Images/Loading.JPG");
}

QIcon   QFileThumbnailProvider::icon(IconType type) const
{
//    qDebug() << __FILE__ << __FUNCTION__ << "icon(IconType type)" << type;
    if (type == QFileIconProvider::File)
    {
        QIcon icon;
        // Add the pixmap to the icon
        icon.addPixmap(*pixmapTemp);
        return icon;
    }

    return QFileIconProvider::icon(type);
}

QIcon   QFileThumbnailProvider::icon(const QFileInfo &info) const
{
    QIcon icon;
    QPixmap pixmap;
    static QTime timer1;

//    qDebug() << __FILE__ << __FUNCTION__ << "Start" << timer1.restart();

    // Check for and if necessary create a cache directory
    QString cacheDir = info.absolutePath() + "/.Cache";
    QDir dir(cacheDir);
    if (!dir.exists())
    {
        // Cache folder doesn't exist so create it
        dir.mkpath(cacheDir);
    }

    // Figure out the cache filename
    QFileInfo cacheFileInfo(cacheDir + "/cache_" + info.fileName());

//    qDebug() << __FILE__ << __FUNCTION__ << "Orig file " << info.fileName() << info.exists() << info.lastModified();
//    qDebug() << __FILE__ << __FUNCTION__ << "Cache file" << cacheFileInfo.fileName() << cacheFileInfo.exists() << cacheFileInfo.lastModified();

    // Cache the filenames
    QString pictureFileName = info.absoluteFilePath();
    QString cacheFileName = cacheFileInfo.absoluteFilePath();

//    qDebug() << __FILE__ << __FUNCTION__ << "Load pixmap" << timer1.restart();

#ifdef CHECK_ISFILE
    if (!info.isFile())
    {
        // This is not a file so just use the default icon
        icon = QFileIconProvider::icon(info);
    }
    else
    {
#endif // CHECK_ISFILE
        if (!cacheFileInfo.exists() || info.lastModified() > cacheFileInfo.lastModified())
        {
            // Cache is older or doesn't exist
            // Check if we are already processing a thumbnail for this one.
            if (!processingThumbnails.contains(pictureFileName))
            {
//                qDebug() << __FILE__ << __FUNCTION__ << "Scheduling " << pictureFileName;

                // Add it to the list of processing so we don't double dip
                processingThumbnails.append(pictureFileName);
#if 1
                // Use the global thread pool
                QFuture<bool> result = QtConcurrent::run(createThumbnail, pictureFileName, cacheFileName, &processingThumbnails, timer);

                // Load the default thumbnail for now
                // pixmap = QFileIconProvider::icon(info).pixmap(QSize(USE_SCALED_ICON_WIDTH,USE_SCALED_ICON_WIDTH * 3 / 5));

                // Add the pixmap to the icon
                icon.addPixmap(*pixmapTemp);
#else
                // Just create the thumbnail in line and wait for it
                if (createThumbnail(pictureFileName, cacheFileName, &processingThumbnails, timer))
                {
                    // Cache created so load it
                    pixmap.load(cacheFileInfo.absoluteFilePath());
                }
                else
                {
                    // loading the pixmap failed so use a standard icon
                    pixmap = QFileIconProvider::icon(info).pixmap(QSize(USE_SCALED_ICON_WIDTH,USE_SCALED_ICON_WIDTH * 3 / 5));
                }

                // Add the pixmap to the icon
                icon.addPixmap(pixmap);
#endif
            }
        }
        else
        {
            // Cache exists and is valid so just load it
            pixmap.load(cacheFileInfo.absoluteFilePath());

            // Add the pixmap to the icon
            icon.addPixmap(pixmap);
        }
#ifdef CHECK_ISFILE
    }
#endif // CHECK_ISFILE

//    qDebug() << __FILE__ << __FUNCTION__ << "Done" << timer1.restart();
    return icon;
}

QString QFileThumbnailProvider::type(const QFileInfo &info) const
{
//    qDebug() << __FILE__ << __FUNCTION__ << "type(const QFileInfo &info)" << info.fileName();
    QString str = QFileIconProvider::type(info);
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return str;
}

void QFileThumbnailProvider::setTimer(QTimer *timer)
{
    this->timer = timer;
}
