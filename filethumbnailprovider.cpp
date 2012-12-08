#include "filethumbnailprovider.h"

#include "myicon.h"

#include <QDebug>

#define EnableThumbnail 1

QFileThumbnailProvider::QFileThumbnailProvider()
{
}

QIcon   QFileThumbnailProvider::icon(IconType type) const
{
    //qDebug() << __FILE__ << __FUNCTION__ << "icon(IconType type)" << type;
    return QFileIconProvider::icon(type);
}

QIcon   QFileThumbnailProvider::icon(const QFileInfo &info) const
{
    qDebug() << __FILE__ << __FUNCTION__ << "icon(const QFileInfo &info)" << info.fileName();
#if EnableThumbnail
    QMyIcon icon(info.absoluteFilePath());
    return icon;
#else
    return QFileIconProvider::icon(info);
#endif
}

QString QFileThumbnailProvider::type(const QFileInfo &info) const
{
    //qDebug() << __FILE__ << __FUNCTION__ << "type(const QFileInfo &info)" << info.fileName();
    return QFileIconProvider::type(info);
}
