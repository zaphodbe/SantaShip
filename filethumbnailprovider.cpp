#include "filethumbnailprovider.h"

#include <QDebug>

#define EnableThumbnail 1

QFileThumbnailProvider::QFileThumbnailProvider()
{
}

QIcon   QFileThumbnailProvider::icon(IconType type) const
{
    //qDebug() << "icon(IconType type)" << type;
    return QFileIconProvider::icon(type);
}

QIcon   QFileThumbnailProvider::icon(const QFileInfo &info) const
{
#if EnableThumbnail
    QIcon icon(info.absoluteFilePath());
    return icon;
#else
    //qDebug() << "icon(const QFileInfo &info)" << info.fileName();
    return QFileIconProvider::icon(info);
#endif
}

QString QFileThumbnailProvider::type(const QFileInfo &info) const
{
    //qDebug() << "type(const QFileInfo &info)" << info.fileName();
    return QFileIconProvider::type(info);
}
