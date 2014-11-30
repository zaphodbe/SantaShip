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

    void setTimer(QTimer *timer);

private:
    QTimer          *timer;
    QPixmap         *pixmapTemp;
};

#endif // FILETHUMBNAILPROVIDER_H
