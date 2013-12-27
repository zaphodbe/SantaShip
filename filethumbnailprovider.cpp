#include "filethumbnailprovider.h"

#include <QDebug>
#include <QDir>
#include <QTime>
#include <QStringList>

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

// Make sure we start off with no thread so it gets created correctly
MyThumbnailThread *QFileThumbnailProvider::m_thumbnailThread = NULL;

/* This is the re-implementation to handle thumbnails in the icons */
QFileThumbnailProvider::QFileThumbnailProvider()
{
    if (!m_thumbnailThread)
    {
        m_thumbnailThread = new MyThumbnailThread();
        m_thumbnailThread->start();
    }
}

QIcon   QFileThumbnailProvider::icon(IconType type) const
{
//    qDebug() << __FILE__ << __FUNCTION__ << "icon(IconType type)" << type;
    QIcon icon = QFileIconProvider::icon(type);
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return icon;
}

QIcon   QFileThumbnailProvider::icon(const QFileInfo &info) const
{
    QIcon icon;
    QPixmap pixmap;
    static QTime timer1;

    qDebug() << __FILE__ << __FUNCTION__ << "Start" << timer1.restart();

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

    qDebug() << __FILE__ << __FUNCTION__ << "Load pixmap" << timer1.restart();

    if (!info.isFile())
    {
        // This is not a file so just use the default icon
        icon = QFileIconProvider::icon(info);
    }
    else
    {
        if (!cacheFileInfo.exists() || info.lastModified() > cacheFileInfo.lastModified())
        {
#if 1
            // Queue the file to be thumbnailed.
            m_thumbnailThread->queuePicture(info.absoluteFilePath(), cacheFileInfo.absoluteFilePath());

            // Load the default thumbnail for now
            pixmap = QFileIconProvider::icon(info).pixmap(QSize(USE_SCALED_ICON_WIDTH,USE_SCALED_ICON_WIDTH * 3 / 5));
#else
            // Cache is older or doesn't exist
            if (pixmap.load(info.absoluteFilePath()))
            {
                pixmap = pixmap.scaledToWidth(USE_SCALED_ICON_WIDTH);
                pixmap.save(cacheFileInfo.absoluteFilePath());
            }
            else
            {
                // loading the pixmap failed so use a standard icon
                pixmap = QFileIconProvider::icon(info).pixmap(QSize(USE_SCALED_ICON_WIDTH,USE_SCALED_ICON_WIDTH * 3 / 5));
            }
#endif
        }
        else
        {
            // Cache exists and is valid so just load it
            pixmap.load(cacheFileInfo.absoluteFilePath());
        }

        // Add the pixmap to the icon
        icon.addPixmap(pixmap);
    }

    qDebug() << __FILE__ << __FUNCTION__ << "Done" << timer1.restart();
    return icon;
}

QString QFileThumbnailProvider::type(const QFileInfo &info) const
{
//    qDebug() << __FILE__ << __FUNCTION__ << "type(const QFileInfo &info)" << info.fileName();
    QString str = QFileIconProvider::type(info);
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return str;
}
