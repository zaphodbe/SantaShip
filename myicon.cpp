#include "myicon.h"

#include <QDebug>
#include <QPixmap>
#include <QTime>
#include <QFileIconProvider>
#include <QDir>

#ifndef USE_SCALED_ICON_WIDTH
#define USE_SCALED_ICON_WIDTH 250
#endif

QMyIcon::QMyIcon(const QFileInfo &info)
{
    static QTime timer1;
    QString cacheDir = info.absolutePath() + "/.Cache";

    QDir dir(cacheDir);
    if (!dir.exists())
    {
        // Cache folder doesn't exist so create it
        dir.mkpath(cacheDir);
    }

    QFileInfo cacheFileInfo(cacheDir + "/cache_" + info.fileName());

//    qDebug() << __FILE__ << __FUNCTION__ << "Orig file " << info.fileName() << info.exists() << info.created();
//    qDebug() << __FILE__ << __FUNCTION__ << "Cache file" << cacheFileInfo.fileName() << cacheFileInfo.exists() << cacheFileInfo.created();

    _fileName = info.absoluteFilePath();
//    qDebug() << __FILE__ << __FUNCTION__ << _fileName << timer1.restart();

    if (!info.isFile())
    {
//        QFileIconProvider::icon(info);
    }
    else
    {
        QPixmap pixmap;
        if (!cacheFileInfo.exists() || info.created() > cacheFileInfo.created())
        {
            // Cache is older or doesn't exist
            if (pixmap.load(info.absoluteFilePath()))
            {
                pixmap = pixmap.scaledToWidth(USE_SCALED_ICON_WIDTH);
                pixmap.save(cacheFileInfo.absoluteFilePath());
            }
            else
            {
                // loading the pixmap failed
                // ToDo: Do Something
            }
        }
        else
        {
            // Cache exists and is valid so just load it
            pixmap.load(cacheFileInfo.absoluteFilePath());
        }

        // Add the pixmap to the icon
        addPixmap(pixmap);
    }

//    qDebug() << __FILE__ << __FUNCTION__ << "done" << timer1.restart();
}

QMyIcon::~QMyIcon()
{
//    qDebug() << __FILE__ << __FUNCTION__ << _fileName;
}
