#ifndef FILETHUMBNAILPROVIDER_H
#define FILETHUMBNAILPROVIDER_H

#include <QFileIconProvider>
#include "mythumbnailthread.h"

class QFileThumbnailProvider : public QFileIconProvider
{
public:
    QFileThumbnailProvider();

    QIcon   icon(IconType type) const;
    QIcon   icon(const QFileInfo &info) const;
    QString type(const QFileInfo &info) const;

private:
    static MyThumbnailThread    *m_thumbnailThread;
};

#endif // FILETHUMBNAILPROVIDER_H
