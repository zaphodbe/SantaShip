#include "filethumbnailprovider.h"

#include "myicon.h"

#include <QDebug>

#define EnableThumbnail 1

QFileThumbnailProvider::QFileThumbnailProvider()
{
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
//    qDebug() << __FILE__ << __FUNCTION__ << "icon(const QFileInfo &info)" << info.fileName();
#if EnableThumbnail
    QMyIcon icon(info.absoluteFilePath());
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return icon;
#else
    QIcon icon = QFileIconProvider::icon(info);
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return icon;
#endif
}

QString QFileThumbnailProvider::type(const QFileInfo &info) const
{
//    qDebug() << __FILE__ << __FUNCTION__ << "type(const QFileInfo &info)" << info.fileName();
    QString str = QFileIconProvider::type(info);
//    qDebug() << __FILE__ << __FUNCTION__ << "done";
    return str;
}
