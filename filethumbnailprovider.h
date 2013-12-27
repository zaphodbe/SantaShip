#ifndef FILETHUMBNAILPROVIDER_H
#define FILETHUMBNAILPROVIDER_H

#include <QFileIconProvider>

class QFileThumbnailProvider : public QFileIconProvider
{
public:
    QFileThumbnailProvider();

    QIcon   icon(IconType type) const;
    QIcon   icon(const QFileInfo &info) const;
    QString type(const QFileInfo &info) const;

private:
};

#endif // FILETHUMBNAILPROVIDER_H
